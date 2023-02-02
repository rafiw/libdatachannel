
#if RTC_ENABLE_MEDIA

#include "rtc/fecrtpcreator.hpp"
#include "byte_io.hpp"
#include "impl/internals.hpp"
#include "message.hpp"
#include "rtc/fecinternal.hpp"
#include "rtp.hpp"
#include "utils.hpp"
#include <cassert>
#include <cmath>

namespace rtc {

constexpr int MAX_FEC_PACKETS = 48;
constexpr int MAX_TRACKED_MEDIA_PACKETS = 4 * MAX_FEC_PACKETS;
// hard code 1500 as max size should be enough
constexpr size_t IP_PACKET_SIZE = 1500;

// Size (in bytes) of packet masks, given number of K bits set.
constexpr size_t kFlexfecPacketMaskSizes[] = {2, 6, 14};

// Size (in bytes) of part of header which is not packet mask specific.
constexpr size_t kBaseHeaderSize = 12;

constexpr size_t kRtpHeaderSize = 12;

// Size (in bytes) of part of header which is stream specific.
constexpr size_t kStreamSpecificHeaderSize = 6;

// Size (in bytes) of header, given the single stream packet mask size, i.e.
// the number of K-bits set.
constexpr size_t kHeaderSizes[] = {
    kBaseHeaderSize + kStreamSpecificHeaderSize + kFlexfecPacketMaskSizes[0],
    kBaseHeaderSize + kStreamSpecificHeaderSize + kFlexfecPacketMaskSizes[1],
    kBaseHeaderSize + kStreamSpecificHeaderSize + kFlexfecPacketMaskSizes[2]};

// TODO(brandtr): Update this when we support multistream protection.
constexpr size_t kPacketMaskOffset = kBaseHeaderSize + kStreamSpecificHeaderSize;

FecRtpCreator::FecRtpCreator(uint32_t synchronizationSource, uint32_t fecPercentage)
    : MediaHandlerElement(), _synchronizationSource(synchronizationSource),
      _fecPercentage(fecPercentage), packet_mask_size_(0), seq_num_base_(0) {
	initFecPackets(MAX_FEC_PACKETS);
}

void FecRtpCreator::initFecPackets(uint32_t num_of_fec_packets) {
	for (size_t i = _fecMessages.size(); i < num_of_fec_packets; i++) {
		auto msg = make_message(IP_PACKET_SIZE);
		RtpHeader *rtp = reinterpret_cast<RtpHeader *>(msg->data());
		rtp->preparePacket();
		rtp->setPayloadType(92);
		rtp->setSsrc(_synchronizationSource);
		_fecMessages.push_back(std::move(msg));
	}
}

size_t FecRtpCreator::NumFecPackets(size_t num_media_packets) {
	// Result in Q0 with an unsigned round.
	uint32_t num_fec_packets = (num_media_packets * _fecPercentage + (1 << 7)) >> 8;
	// Generate at least one FEC packet if we need protection.
	if (_fecPercentage > 0 && num_fec_packets == 0) {
		num_fec_packets = 1;
	}
	return std::min<size_t>(num_fec_packets, MAX_FEC_PACKETS);
}

void FecRtpCreator::XorHeaders(const binary_ptr src, message_ptr dst) {
	auto dst_data = dst->data() + kRtpHeaderSize;
	const auto src_data = src->data();
	// XOR the first 2 bytes of the header: V, P, X, CC, M, PT fields.
	dst_data[0] ^= src_data[0];
	dst_data[1] ^= src_data[1];

	// XOR the length recovery field.
	dst_data[2] ^= (std::byte)((src->size() - 12) >> 8);
	dst_data[3] ^= (std::byte)((src->size() - 12) & 0x00ff);

	// XOR the 5th to 8th bytes of the header: the timestamp field.
	dst_data[4] ^= src_data[4];
	dst_data[5] ^= src_data[5];
	dst_data[6] ^= src_data[6];
	dst_data[7] ^= src_data[7];

	// Skip the 9th to 12th bytes of the header.
}

void FecRtpCreator::XorPayloads(const binary_ptr src, size_t payload_length, size_t dst_offset,
                                message_ptr dst) {
	// XOR the payload.
	if (dst_offset + payload_length > dst->size()) {
		size_t old_size = dst->size();
		size_t new_size = dst_offset + payload_length;
		dst->resize(new_size);
		std::fill(dst->begin() + old_size, dst->end(), std::byte());
	}
	auto dst_data = dst->data() + kRtpHeaderSize;
	const std::byte *src_data = src->data();
	for (size_t i = 0; i < payload_length; ++i) {
		dst_data[dst_offset + i] ^= src_data[12 + i];
	}
}

int FecRtpCreator::InsertZerosInPacketMasks(ChainedMessagesProduct media_packets,
                                            size_t num_fec_packets) {
	size_t num_media_packets = media_packets->size();
	if (num_media_packets <= 1) {
		return num_media_packets;
	}
	uint16_t last_seq_num =
	    reinterpret_cast<RtpHeader *>(media_packets->back()->data())->seqNumber();
	uint16_t first_seq_num =
	    reinterpret_cast<RtpHeader *>(media_packets->front()->data())->seqNumber();
	size_t total_missing_seq_nums =
	    static_cast<uint16_t>(last_seq_num - first_seq_num) - num_media_packets + 1;
	if (total_missing_seq_nums == 0) {
		// All sequence numbers are covered by the packet mask.
		// No zero insertion required.
		return num_media_packets;
	}
	const size_t max_media_packets = MAX_TRACKED_MEDIA_PACKETS;
	if (total_missing_seq_nums + num_media_packets > max_media_packets) {
		return -1;
	}
	// Allocate the new mask.
	size_t tmp_packet_mask_size =
	    internal::PacketMaskSize(total_missing_seq_nums + num_media_packets);
	memset(tmp_packet_masks_, 0, num_fec_packets * tmp_packet_mask_size);

	auto media_packets_it = media_packets->cbegin();
	uint16_t prev_seq_num = first_seq_num;
	++media_packets_it;

	// Insert the first column.
	internal::CopyColumn(tmp_packet_masks_, tmp_packet_mask_size, packet_masks_, packet_mask_size_,
	                     num_fec_packets, 0, 0);
	size_t new_bit_index = 1;
	size_t old_bit_index = 1;
	// Insert zeros in the bit mask for every hole in the sequence.
	while (media_packets_it != media_packets->end()) {
		if (new_bit_index == max_media_packets) {
			// We can only cover up to 48 packets.
			break;
		}
		uint16_t seq_num = reinterpret_cast<RtpHeader *>((*media_packets_it)->data())->seqNumber();
		const int num_zeros_to_insert = static_cast<uint16_t>(seq_num - prev_seq_num - 1);
		if (num_zeros_to_insert > 0) {
			internal::InsertZeroColumns(num_zeros_to_insert, tmp_packet_masks_,
			                            tmp_packet_mask_size, num_fec_packets, new_bit_index);
		}
		new_bit_index += num_zeros_to_insert;
		internal::CopyColumn(tmp_packet_masks_, tmp_packet_mask_size, packet_masks_,
		                     packet_mask_size_, num_fec_packets, new_bit_index, old_bit_index);
		++new_bit_index;
		++old_bit_index;
		prev_seq_num = seq_num;
		++media_packets_it;
	}
	if (new_bit_index % 8 != 0) {
		// We didn't fill the last byte. Shift bits to correct position.
		for (uint16_t row = 0; row < num_fec_packets; ++row) {
			int new_byte_index = row * tmp_packet_mask_size + new_bit_index / 8;
			tmp_packet_masks_[new_byte_index] <<= (7 - (new_bit_index % 8));
		}
	}
	// Replace the old mask with the new.
	memcpy(packet_masks_, tmp_packet_masks_, num_fec_packets * tmp_packet_mask_size);
	return new_bit_index;
}

size_t FecRtpCreator::MinPacketMaskSize(const uint8_t *packet_mask, size_t packet_mask_size) const {
	if (packet_mask_size == kUlpfecPacketMaskSizeLBitClear && (packet_mask[1] & 0x01) == 0) {
		// Packet mask is 16 bits long, with bit 15 clear.
		// It can be used as is.
		return kFlexfecPacketMaskSizes[0];
	} else if (packet_mask_size == kUlpfecPacketMaskSizeLBitClear) {
		// Packet mask is 16 bits long, with bit 15 set.
		// We must expand the packet mask with zeros in the FlexFEC header.
		return kFlexfecPacketMaskSizes[1];
	} else if (packet_mask_size == kUlpfecPacketMaskSizeLBitSet && (packet_mask[5] & 0x03) == 0) {
		// Packet mask is 48 bits long, with bits 46 and 47 clear.
		// It can be used as is.
		return kFlexfecPacketMaskSizes[1];
	} else if (packet_mask_size == kUlpfecPacketMaskSizeLBitSet) {
		// Packet mask is 48 bits long, with at least one of bits 46 and 47 set.
		// We must expand it with zeros.
		return kFlexfecPacketMaskSizes[2];
	}
	return kFlexfecPacketMaskSizes[2];
}

size_t FecRtpCreator::FlexfecHeaderSize(size_t packet_mask_size) {
	if (packet_mask_size <= kFlexfecPacketMaskSizes[0]) {
		return kHeaderSizes[0];
	} else if (packet_mask_size <= kFlexfecPacketMaskSizes[1]) {
		return kHeaderSizes[1];
	}
	return kHeaderSizes[2];
}

void FecRtpCreator::GenerateFecPayloads(ChainedMessagesProduct media_packets,
                                        size_t num_fec_packets) {
	for (size_t i = 0; i < num_fec_packets; ++i) {
		auto fec_packet = _fecMessages[i];
		size_t pkt_mask_idx = i * packet_mask_size_;
		const size_t min_packet_mask_size =
		    MinPacketMaskSize(&packet_masks_[pkt_mask_idx], packet_mask_size_);
		const size_t fec_header_size = FlexfecHeaderSize(min_packet_mask_size);

		size_t media_pkt_idx = 0;
		auto media_packets_it = media_packets->cbegin();
		uint16_t prev_seq_num =
		    reinterpret_cast<RtpHeader *>((*media_packets_it)->data())->seqNumber();
		while (media_packets_it != media_packets->end()) {
			auto media_packet = *media_packets_it;
			// Should `media_packet` be protected by `fec_packet`?
			if (packet_masks_[pkt_mask_idx] & (1 << (7 - media_pkt_idx))) {
				size_t media_payload_length = media_packet->size() - kRtpHeaderSize;

				size_t fec_packet_length = kRtpHeaderSize + fec_header_size + media_payload_length;
				fec_packet->resize(fec_packet_length);
				// if (fec_packet_length > fec_packet->size()) {
				// 	PLOG_ERROR << "Fec packet too large encreasing to " << fec_packet_length;
				// 	size_t old_size = fec_packet->size();
				// 	fec_packet->resize(fec_packet_length);
				// 	memset(fec_packet->data() + old_size, 0, fec_packet_length - old_size);
				// }
				XorHeaders(media_packet, fec_packet);
				XorPayloads(media_packet, media_payload_length, fec_header_size, fec_packet);
			}
			media_packets_it++;
			if (media_packets_it != media_packets->end()) {
				uint16_t seq_num =
				    reinterpret_cast<RtpHeader *>((*media_packets_it)->data())->seqNumber();
				media_pkt_idx += static_cast<uint16_t>(seq_num - prev_seq_num);
				prev_seq_num = seq_num;
			}
			pkt_mask_idx += media_pkt_idx / 8;
			media_pkt_idx %= 8;
		}
	}
}

void FecRtpCreator::FinalizeFecHeaders(size_t num_fec_packets, uint32_t media_ssrc,
                                       uint16_t seq_num_base) {
	// We currently only support single-stream protection.
	// TODO(brandtr): Update this when we support multistream protection.
	constexpr uint8_t kSsrcCount = 1;
	// There are three reserved bytes that MUST be set to zero in the header.
	constexpr uint32_t kReservedBits = 0;
	for (size_t i = 0; i < num_fec_packets; ++i) {
		const uint8_t *packet_mask = &packet_masks_[i * packet_mask_size_];
		std::byte *fecData = _fecMessages[i]->data() + kRtpHeaderSize;

		fecData[0] &= std::byte(0x7f); // Clear R bit.
		fecData[0] &= std::byte(0xbf); // Clear F bit.
		webrtc::ByteWriter<uint8_t>::WriteBigEndian(&fecData[8], kSsrcCount);
		webrtc::ByteWriter<uint32_t, 3>::WriteBigEndian(&fecData[9], kReservedBits);
		webrtc::ByteWriter<uint32_t>::WriteBigEndian(&fecData[12], media_ssrc);
		webrtc::ByteWriter<uint16_t>::WriteBigEndian(&fecData[16], seq_num_base);
		// Adapt ULPFEC packet mask to FlexFEC header.
		//
		// We treat the mask parts as unsigned integers with host order endianness
		// in order to simplify the bit shifting between bytes.
		auto *const written_packet_mask = fecData + kPacketMaskOffset;
		if (packet_mask_size_ == kUlpfecPacketMaskSizeLBitSet) {
			// The packet mask is 48 bits long.
			uint16_t tmp_mask_part0 = webrtc::ByteReader<uint16_t>::ReadBigEndian(&packet_mask[0]);
			uint32_t tmp_mask_part1 = webrtc::ByteReader<uint32_t>::ReadBigEndian(&packet_mask[2]);

			tmp_mask_part0 >>= 1; // Shift, thus clearing K-bit 0.
			webrtc::ByteWriter<uint16_t>::WriteBigEndian(&written_packet_mask[0], tmp_mask_part0);
			tmp_mask_part1 >>= 2; // Shift, thus clearing K-bit 1 and bit 15.
			webrtc::ByteWriter<uint32_t>::WriteBigEndian(&written_packet_mask[2], tmp_mask_part1);
			bool bit15 = (packet_mask[1] & 0x01) != 0;
			if (bit15)
				written_packet_mask[2] |= std::byte(0x40); // Set bit 15.
			bool bit46 = (packet_mask[5] & 0x02) != 0;
			bool bit47 = (packet_mask[5] & 0x01) != 0;
			if (!bit46 && !bit47) {
				written_packet_mask[2] |= std::byte(0x80); // Set K-bit 1.
			} else {
				memset(&written_packet_mask[6], 0, 8);     // Clear all trailing bits.
				written_packet_mask[6] |= std::byte(0x80); // Set K-bit 2.
				if (bit46)
					written_packet_mask[6] |= std::byte(0x40); // Set bit 46.
				if (bit47)
					written_packet_mask[6] |= std::byte(0x20); // Set bit 47.
			}
		} else if (packet_mask_size_ == kUlpfecPacketMaskSizeLBitClear) {
			// The packet mask is 16 bits long.
			uint16_t tmp_mask_part0 = webrtc::ByteReader<uint16_t>::ReadBigEndian(&packet_mask[0]);

			tmp_mask_part0 >>= 1; // Shift, thus clearing K-bit 0.
			webrtc::ByteWriter<uint16_t>::WriteBigEndian(&written_packet_mask[0], tmp_mask_part0);
			bool bit15 = (packet_mask[1] & 0x01) != 0;
			if (!bit15) {
				written_packet_mask[0] |= std::byte(0x80); // Set K-bit 0.
			} else {
				memset(&written_packet_mask[2], 0U, 4);    // Clear all trailing bits.
				written_packet_mask[2] |= std::byte(0x80); // Set K-bit 1.
				written_packet_mask[2] |= std::byte(0x40); // Set bit 15.
			}
		}
	}
}

void FecRtpCreator::FinalizeRtpHeader(message_ptr fec_packet,
                                      shared_ptr<std::vector<std::byte>> media_packet) {
	auto *srcRtp = (RtpHeader *)media_packet->data();
	auto *rtp = (RtpHeader *)fec_packet->data();
	rtp->preparePacket();
	rtp->setMarker(false);
	rtp->setPayloadType(srcRtp->payloadType());
	rtp->setTimestamp(srcRtp->timestamp());
	rtp->setSeqNumber(seq_num_base_++);
}

ChainedOutgoingProduct FecRtpCreator::processOutgoingBinaryMessage(ChainedMessagesProduct messages,
                                                                   message_ptr control) {
	if (messages->size() == 0)
		return {messages, control};
	// auto firstRtpHeader = reinterpret_cast<RtpHeader*>(messages->front()->data());
	// auto lastRtpHeader = reinterpret_cast<RtpHeader*>(messages->back()->data());
	// auto firstSeq = firstRtpHeader->seqNumber();
	auto packetsNum = messages->size();

	uint32_t num_fec_packets = NumFecPackets(packetsNum);
	if (num_fec_packets == 0) {
		return {messages, control};
	}
	if (num_fec_packets > _fecMessages.size()) {
		initFecPackets(num_fec_packets);
	}
	for (size_t i = 0; i < num_fec_packets; ++i) {
		// memset(_fecMessages[i]->data(), 0, _fecMessages[i]->size());
		std::fill(_fecMessages[i]->begin() + kRtpHeaderSize, _fecMessages[i]->end(), std::byte{0});
	}
	internal::PacketMaskTable mask_table((int)packetsNum);
	size_t packet_mask_size_ = internal::PacketMaskSize(packetsNum);
	memset(packet_masks_, 0, num_fec_packets * packet_mask_size_);
	internal::GeneratePacketMasks(packetsNum, num_fec_packets, &mask_table, packet_masks_);
	int numMaskBits = InsertZerosInPacketMasks(messages, num_fec_packets);
	if (numMaskBits < 0) {
		return {messages, control};
	}
	packet_mask_size_ = internal::PacketMaskSize(numMaskBits);
	GenerateFecPayloads(messages, num_fec_packets);
	const uint32_t media_ssrc = reinterpret_cast<RtpHeader *>(messages->front()->data())->ssrc();
	const uint16_t seq_num_base =
	    reinterpret_cast<RtpHeader *>(messages->front()->data())->seqNumber();
	FinalizeFecHeaders(num_fec_packets, media_ssrc, seq_num_base);

	// copy packets to end of messages
	std::vector<std::byte> media = *messages->front();
	std::vector<std::byte> fec = *this->_fecMessages[0];
	for (size_t i = 0; i < num_fec_packets; ++i) {
		FinalizeRtpHeader(this->_fecMessages[i], messages->front());
		messages->push_back(this->_fecMessages[i]);
	}
	// PLOG_INFO << "sent " << num_fec_packets;
	return {messages, control};
}

} // namespace rtc

#endif /* RTC_ENABLE_MEDIA */
