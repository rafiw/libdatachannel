#ifndef RTC_FEC_CREATOR_H
#define RTC_FEC_CREATOR_H

#if RTC_ENABLE_MEDIA

#include "fecinternal.hpp"
#include "mediahandlerelement.hpp"
#include <queue>
#include <unordered_map>

constexpr int FEC_DEFAULT_PERCENTAGE = 10;
constexpr size_t kUlpfecMaxMediaPackets = 48;
constexpr size_t kUlpfecMaxPacketMaskSize = kUlpfecPacketMaskSizeLBitSet;

namespace rtc {

class RTC_CPP_EXPORT FecRtpCreator final : public MediaHandlerElement {
	void initFecPackets(uint32_t num_of_fec_packets);

public:
	FecRtpCreator(uint32_t synchronizationSource, uint32_t fecPercentage = FEC_DEFAULT_PERCENTAGE);

	/// Stores RTP packets in internal storage
	/// @param messages RTP packets
	/// @param control RTCP
	/// @returns Unchanged RTP and RTCP
	ChainedOutgoingProduct processOutgoingBinaryMessage(ChainedMessagesProduct messages,
	                                                    message_ptr control) override;

private:
	void XorHeaders(const binary_ptr src, message_ptr dst);
	void XorPayloads(const binary_ptr src, size_t payload_length, size_t dst_offset,
	                 message_ptr dst);
	int InsertZerosInPacketMasks(ChainedMessagesProduct messages, size_t num_fec_packets);
	void GenerateFecPayloads(ChainedMessagesProduct media_packets, size_t num_fec_packets);
	size_t MinPacketMaskSize(const uint8_t *packet_mask, size_t packet_mask_size) const;
	size_t NumFecPackets(size_t num_media_packets);
	size_t FlexfecHeaderSize(size_t packet_mask_size);
	void FinalizeFecHeaders(size_t num_fec_packets, uint32_t media_ssrc, uint16_t seq_num_base);
	void FinalizeRtpHeader(message_ptr fec_packet, shared_ptr<std::vector<std::byte>> media_packet);
	std::vector<message_ptr> _fecMessages;
	uint32_t _synchronizationSource;
	uint32_t _fecPercentage;
	uint8_t packet_masks_[kUlpfecMaxMediaPackets * kUlpfecMaxPacketMaskSize];
	uint8_t tmp_packet_masks_[kUlpfecMaxMediaPackets * kUlpfecMaxPacketMaskSize];
	size_t packet_mask_size_;
	uint16_t seq_num_base_ = 0;
};

} // namespace rtc

#endif /* RTC_ENABLE_MEDIA */

#endif /* RTC_FEC_CREATOR_H */
