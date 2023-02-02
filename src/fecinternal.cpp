/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
/*
this code was copied from webrtc source code to experiment adding FlexFEC support
*/
#include "rtc/fecinternal.hpp"

namespace {
// clang-format off
#define kMaskBursty1_1 \
  0x80, 0x00

#define kMaskBursty2_1 \
  0xc0, 0x00

#define kMaskBursty2_2 \
  0x80, 0x00, \
  0xc0, 0x00

#define kMaskBursty3_1 \
  0xe0, 0x00

#define kMaskBursty3_2 \
  0xc0, 0x00, \
  0xa0, 0x00

#define kMaskBursty3_3 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00

#define kMaskBursty4_1 \
  0xf0, 0x00

#define kMaskBursty4_2 \
  0xa0, 0x00, \
  0xd0, 0x00

#define kMaskBursty4_3 \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x90, 0x00

#define kMaskBursty4_4 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00

#define kMaskBursty5_1 \
  0xf8, 0x00

#define kMaskBursty5_2 \
  0xd0, 0x00, \
  0xa8, 0x00

#define kMaskBursty5_3 \
  0x70, 0x00, \
  0x90, 0x00, \
  0xc8, 0x00

#define kMaskBursty5_4 \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x88, 0x00

#define kMaskBursty5_5 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00

#define kMaskBursty6_1 \
  0xfc, 0x00

#define kMaskBursty6_2 \
  0xa8, 0x00, \
  0xd4, 0x00

#define kMaskBursty6_3 \
  0x94, 0x00, \
  0xc8, 0x00, \
  0x64, 0x00

#define kMaskBursty6_4 \
  0x60, 0x00, \
  0x38, 0x00, \
  0x88, 0x00, \
  0xc4, 0x00

#define kMaskBursty6_5 \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x84, 0x00

#define kMaskBursty6_6 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00

#define kMaskBursty7_1 \
  0xfe, 0x00

#define kMaskBursty7_2 \
  0xd4, 0x00, \
  0xaa, 0x00

#define kMaskBursty7_3 \
  0xc8, 0x00, \
  0x74, 0x00, \
  0x92, 0x00

#define kMaskBursty7_4 \
  0x38, 0x00, \
  0x8a, 0x00, \
  0xc4, 0x00, \
  0x62, 0x00

#define kMaskBursty7_5 \
  0x60, 0x00, \
  0x30, 0x00, \
  0x1c, 0x00, \
  0x84, 0x00, \
  0xc2, 0x00

#define kMaskBursty7_6 \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x82, 0x00

#define kMaskBursty7_7 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00

#define kMaskBursty8_1 \
  0xff, 0x00

#define kMaskBursty8_2 \
  0xaa, 0x00, \
  0xd5, 0x00

#define kMaskBursty8_3 \
  0x74, 0x00, \
  0x92, 0x00, \
  0xc9, 0x00

#define kMaskBursty8_4 \
  0x8a, 0x00, \
  0xc5, 0x00, \
  0x62, 0x00, \
  0x31, 0x00

#define kMaskBursty8_5 \
  0x30, 0x00, \
  0x1c, 0x00, \
  0x85, 0x00, \
  0xc2, 0x00, \
  0x61, 0x00

#define kMaskBursty8_6 \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0e, 0x00, \
  0x82, 0x00, \
  0xc1, 0x00

#define kMaskBursty8_7 \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x81, 0x00

#define kMaskBursty8_8 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00

#define kMaskBursty9_1 \
  0xff, 0x80

#define kMaskBursty9_2 \
  0xd5, 0x00, \
  0xaa, 0x80

#define kMaskBursty9_3 \
  0x92, 0x00, \
  0xc9, 0x00, \
  0x74, 0x80

#define kMaskBursty9_4 \
  0xc5, 0x00, \
  0x62, 0x00, \
  0x39, 0x00, \
  0x8a, 0x80

#define kMaskBursty9_5 \
  0x1c, 0x00, \
  0x85, 0x00, \
  0xc2, 0x80, \
  0x61, 0x00, \
  0x30, 0x80

#define kMaskBursty9_6 \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0e, 0x00, \
  0x82, 0x80, \
  0xc1, 0x00, \
  0x60, 0x80

#define kMaskBursty9_7 \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x07, 0x00, \
  0x81, 0x00, \
  0xc0, 0x80

#define kMaskBursty9_8 \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x80, 0x80

#define kMaskBursty9_9 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80

#define kMaskBursty10_1 \
  0xff, 0xc0

#define kMaskBursty10_2 \
  0xaa, 0x80, \
  0xd5, 0x40

#define kMaskBursty10_3 \
  0xc9, 0x00, \
  0x74, 0x80, \
  0x92, 0x40

#define kMaskBursty10_4 \
  0x62, 0x00, \
  0x39, 0x00, \
  0x8a, 0x80, \
  0xc5, 0x40

#define kMaskBursty10_5 \
  0x85, 0x00, \
  0xc2, 0x80, \
  0x61, 0x40, \
  0x30, 0x80, \
  0x18, 0x40

#define kMaskBursty10_6 \
  0x18, 0x00, \
  0x0e, 0x00, \
  0x82, 0x80, \
  0xc1, 0x40, \
  0x60, 0x80, \
  0x30, 0x40

#define kMaskBursty10_7 \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x07, 0x00, \
  0x81, 0x40, \
  0xc0, 0x80, \
  0x60, 0x40

#define kMaskBursty10_8 \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x80, 0x80, \
  0xc0, 0x40

#define kMaskBursty10_9 \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80, \
  0x80, 0x40

#define kMaskBursty10_10 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80, \
  0x00, 0xc0

#define kMaskBursty11_1 \
  0xff, 0xe0

#define kMaskBursty11_2 \
  0xd5, 0x40, \
  0xaa, 0xa0

#define kMaskBursty11_3 \
  0x74, 0x80, \
  0x92, 0x40, \
  0xc9, 0x20

#define kMaskBursty11_4 \
  0x39, 0x00, \
  0x8a, 0x80, \
  0xc5, 0x40, \
  0x62, 0x20

#define kMaskBursty11_5 \
  0xc2, 0xc0, \
  0x61, 0x00, \
  0x30, 0xa0, \
  0x1c, 0x40, \
  0x85, 0x20

#define kMaskBursty11_6 \
  0x0e, 0x00, \
  0x82, 0x80, \
  0xc1, 0x40, \
  0x60, 0xa0, \
  0x30, 0x40, \
  0x18, 0x20

#define kMaskBursty11_7 \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x07, 0x00, \
  0x81, 0x40, \
  0xc0, 0xa0, \
  0x60, 0x40, \
  0x30, 0x20

#define kMaskBursty11_8 \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x40, \
  0x80, 0xa0, \
  0xc0, 0x40, \
  0x60, 0x20

#define kMaskBursty11_9 \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80, \
  0x80, 0x40, \
  0xc0, 0x20

#define kMaskBursty11_10 \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80, \
  0x00, 0xc0, \
  0x80, 0x20

#define kMaskBursty11_11 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80, \
  0x00, 0xc0, \
  0x00, 0x60

#define kMaskBursty12_1 \
  0xff, 0xf0

#define kMaskBursty12_2 \
  0xaa, 0xa0, \
  0xd5, 0x50

#define kMaskBursty12_3 \
  0x92, 0x40, \
  0xc9, 0x20, \
  0x74, 0x90

#define kMaskBursty12_4 \
  0x8a, 0x80, \
  0xc5, 0x40, \
  0x62, 0x20, \
  0x39, 0x10

#define kMaskBursty12_5 \
  0x61, 0x00, \
  0x30, 0xa0, \
  0x1c, 0x50, \
  0x85, 0x20, \
  0xc2, 0x90

#define kMaskBursty12_6 \
  0x82, 0x90, \
  0xc1, 0x40, \
  0x60, 0xa0, \
  0x30, 0x50, \
  0x18, 0x20, \
  0x0c, 0x10

#define kMaskBursty12_7 \
  0x0c, 0x00, \
  0x07, 0x00, \
  0x81, 0x40, \
  0xc0, 0xa0, \
  0x60, 0x50, \
  0x30, 0x20, \
  0x18, 0x10

#define kMaskBursty12_8 \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x80, 0xa0, \
  0xc0, 0x50, \
  0x60, 0x20, \
  0x30, 0x10

#define kMaskBursty12_9 \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80, \
  0x80, 0x50, \
  0xc0, 0x20, \
  0x60, 0x10

#define kMaskBursty12_10 \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80, \
  0x00, 0xc0, \
  0x80, 0x20, \
  0xc0, 0x10

#define kMaskBursty12_11 \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80, \
  0x00, 0xc0, \
  0x00, 0x60, \
  0x80, 0x10

#define kMaskBursty12_12 \
  0x80, 0x00, \
  0xc0, 0x00, \
  0x60, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0x0c, 0x00, \
  0x06, 0x00, \
  0x03, 0x00, \
  0x01, 0x80, \
  0x00, 0xc0, \
  0x00, 0x60, \
  0x00, 0x30

#define kPacketMaskBursty1 1, \
  kMaskBursty1_1

#define kPacketMaskBursty2 2, \
  kMaskBursty2_1, \
  kMaskBursty2_2

#define kPacketMaskBursty3 3, \
  kMaskBursty3_1, \
  kMaskBursty3_2, \
  kMaskBursty3_3

#define kPacketMaskBursty4 4, \
  kMaskBursty4_1, \
  kMaskBursty4_2, \
  kMaskBursty4_3, \
  kMaskBursty4_4

#define kPacketMaskBursty5 5, \
  kMaskBursty5_1, \
  kMaskBursty5_2, \
  kMaskBursty5_3, \
  kMaskBursty5_4, \
  kMaskBursty5_5

#define kPacketMaskBursty6 6, \
  kMaskBursty6_1, \
  kMaskBursty6_2, \
  kMaskBursty6_3, \
  kMaskBursty6_4, \
  kMaskBursty6_5, \
  kMaskBursty6_6

#define kPacketMaskBursty7 7, \
  kMaskBursty7_1, \
  kMaskBursty7_2, \
  kMaskBursty7_3, \
  kMaskBursty7_4, \
  kMaskBursty7_5, \
  kMaskBursty7_6, \
  kMaskBursty7_7

#define kPacketMaskBursty8 8, \
  kMaskBursty8_1, \
  kMaskBursty8_2, \
  kMaskBursty8_3, \
  kMaskBursty8_4, \
  kMaskBursty8_5, \
  kMaskBursty8_6, \
  kMaskBursty8_7, \
  kMaskBursty8_8

#define kPacketMaskBursty9 9, \
  kMaskBursty9_1, \
  kMaskBursty9_2, \
  kMaskBursty9_3, \
  kMaskBursty9_4, \
  kMaskBursty9_5, \
  kMaskBursty9_6, \
  kMaskBursty9_7, \
  kMaskBursty9_8, \
  kMaskBursty9_9

#define kPacketMaskBursty10 10, \
  kMaskBursty10_1, \
  kMaskBursty10_2, \
  kMaskBursty10_3, \
  kMaskBursty10_4, \
  kMaskBursty10_5, \
  kMaskBursty10_6, \
  kMaskBursty10_7, \
  kMaskBursty10_8, \
  kMaskBursty10_9, \
  kMaskBursty10_10

#define kPacketMaskBursty11 11, \
  kMaskBursty11_1, \
  kMaskBursty11_2, \
  kMaskBursty11_3, \
  kMaskBursty11_4, \
  kMaskBursty11_5, \
  kMaskBursty11_6, \
  kMaskBursty11_7, \
  kMaskBursty11_8, \
  kMaskBursty11_9, \
  kMaskBursty11_10, \
  kMaskBursty11_11

#define kPacketMaskBursty12 12, \
  kMaskBursty12_1, \
  kMaskBursty12_2, \
  kMaskBursty12_3, \
  kMaskBursty12_4, \
  kMaskBursty12_5, \
  kMaskBursty12_6, \
  kMaskBursty12_7, \
  kMaskBursty12_8, \
  kMaskBursty12_9, \
  kMaskBursty12_10, \
  kMaskBursty12_11, \
  kMaskBursty12_12

// clang-format off
#define kMaskRandom1_1 \
  0x80, 0x00

#define kMaskRandom2_1 \
  0xc0, 0x00

#define kMaskRandom2_2 \
  0xc0, 0x00, \
  0x80, 0x00

#define kMaskRandom3_1 \
  0xe0, 0x00

#define kMaskRandom3_2 \
  0xc0, 0x00, \
  0xa0, 0x00

#define kMaskRandom3_3 \
  0xc0, 0x00, \
  0xa0, 0x00, \
  0x60, 0x00

#define kMaskRandom4_1 \
  0xf0, 0x00

#define kMaskRandom4_2 \
  0xc0, 0x00, \
  0xb0, 0x00

#define kMaskRandom4_3 \
  0xc0, 0x00, \
  0xb0, 0x00, \
  0x60, 0x00

#define kMaskRandom4_4 \
  0xc0, 0x00, \
  0xa0, 0x00, \
  0x30, 0x00, \
  0x50, 0x00

#define kMaskRandom5_1 \
  0xf8, 0x00

#define kMaskRandom5_2 \
  0xa8, 0x00, \
  0xd0, 0x00

#define kMaskRandom5_3 \
  0xb0, 0x00, \
  0xc8, 0x00, \
  0x50, 0x00

#define kMaskRandom5_4 \
  0xc8, 0x00, \
  0xb0, 0x00, \
  0x50, 0x00, \
  0x28, 0x00

#define kMaskRandom5_5 \
  0xc0, 0x00, \
  0x30, 0x00, \
  0x18, 0x00, \
  0xa0, 0x00, \
  0x48, 0x00

#define kMaskRandom6_1 \
  0xfc, 0x00

#define kMaskRandom6_2 \
  0xa8, 0x00, \
  0xd4, 0x00

#define kMaskRandom6_3 \
  0xd0, 0x00, \
  0x68, 0x00, \
  0xa4, 0x00

#define kMaskRandom6_4 \
  0xa8, 0x00, \
  0x58, 0x00, \
  0x64, 0x00, \
  0x94, 0x00

#define kMaskRandom6_5 \
  0xa8, 0x00, \
  0x84, 0x00, \
  0x64, 0x00, \
  0x90, 0x00, \
  0x58, 0x00

#define kMaskRandom6_6 \
  0x98, 0x00, \
  0x64, 0x00, \
  0x50, 0x00, \
  0x14, 0x00, \
  0xa8, 0x00, \
  0xe0, 0x00

#define kMaskRandom7_1 \
  0xfe, 0x00

#define kMaskRandom7_2 \
  0xd4, 0x00, \
  0xaa, 0x00

#define kMaskRandom7_3 \
  0xd0, 0x00, \
  0xaa, 0x00, \
  0x64, 0x00

#define kMaskRandom7_4 \
  0xd0, 0x00, \
  0xaa, 0x00, \
  0x64, 0x00, \
  0x1c, 0x00

#define kMaskRandom7_5 \
  0x0c, 0x00, \
  0xb0, 0x00, \
  0x1a, 0x00, \
  0xc4, 0x00, \
  0x62, 0x00

#define kMaskRandom7_6 \
  0x8c, 0x00, \
  0x4a, 0x00, \
  0x64, 0x00, \
  0xd0, 0x00, \
  0xa0, 0x00, \
  0x32, 0x00

#define kMaskRandom7_7 \
  0x4a, 0x00, \
  0x94, 0x00, \
  0x1a, 0x00, \
  0xc4, 0x00, \
  0x28, 0x00, \
  0xc2, 0x00, \
  0x34, 0x00

#define kMaskRandom8_1 \
  0xff, 0x00

#define kMaskRandom8_2 \
  0xaa, 0x00, \
  0xd5, 0x00

#define kMaskRandom8_3 \
  0xc5, 0x00, \
  0x92, 0x00, \
  0x6a, 0x00

#define kMaskRandom8_4 \
  0x45, 0x00, \
  0xb4, 0x00, \
  0x6a, 0x00, \
  0x89, 0x00

#define kMaskRandom8_5 \
  0x8c, 0x00, \
  0x92, 0x00, \
  0x2b, 0x00, \
  0x51, 0x00, \
  0x64, 0x00

#define kMaskRandom8_6 \
  0xa1, 0x00, \
  0x52, 0x00, \
  0x91, 0x00, \
  0x2a, 0x00, \
  0xc4, 0x00, \
  0x4c, 0x00

#define kMaskRandom8_7 \
  0x15, 0x00, \
  0xc2, 0x00, \
  0x25, 0x00, \
  0x62, 0x00, \
  0x58, 0x00, \
  0x8c, 0x00, \
  0xa3, 0x00

#define kMaskRandom8_8 \
  0x25, 0x00, \
  0x8a, 0x00, \
  0x91, 0x00, \
  0x68, 0x00, \
  0x32, 0x00, \
  0x43, 0x00, \
  0xc4, 0x00, \
  0x1c, 0x00

#define kMaskRandom9_1 \
  0xff, 0x80

#define kMaskRandom9_2 \
  0xaa, 0x80, \
  0xd5, 0x00

#define kMaskRandom9_3 \
  0xa5, 0x00, \
  0xc8, 0x00, \
  0x52, 0x80

#define kMaskRandom9_4 \
  0xa2, 0x00, \
  0xc9, 0x00, \
  0x52, 0x80, \
  0x24, 0x80

#define kMaskRandom9_5 \
  0x8c, 0x00, \
  0x25, 0x00, \
  0x92, 0x80, \
  0x41, 0x80, \
  0x58, 0x00

#define kMaskRandom9_6 \
  0x84, 0x80, \
  0x27, 0x00, \
  0x51, 0x80, \
  0x1a, 0x00, \
  0x68, 0x00, \
  0x89, 0x00

#define kMaskRandom9_7 \
  0x8c, 0x00, \
  0x47, 0x00, \
  0x81, 0x80, \
  0x12, 0x80, \
  0x58, 0x00, \
  0x28, 0x80, \
  0xb4, 0x00

#define kMaskRandom9_8 \
  0x2c, 0x00, \
  0x91, 0x00, \
  0x40, 0x80, \
  0x06, 0x80, \
  0xc8, 0x00, \
  0x45, 0x00, \
  0x30, 0x80, \
  0xa2, 0x00

#define kMaskRandom9_9 \
  0x4c, 0x00, \
  0x62, 0x00, \
  0x91, 0x00, \
  0x42, 0x80, \
  0xa4, 0x00, \
  0x13, 0x00, \
  0x30, 0x80, \
  0x88, 0x80, \
  0x09, 0x00

#define kMaskRandom10_1 \
  0xff, 0xc0

#define kMaskRandom10_10 \
  0x4c, 0x00, \
  0x51, 0x00, \
  0xa0, 0x40, \
  0x04, 0xc0, \
  0x03, 0x80, \
  0x86, 0x00, \
  0x29, 0x00, \
  0x42, 0x40, \
  0x98, 0x00, \
  0x30, 0x80

#define kMaskRandom10_2 \
  0xaa, 0x80, \
  0xd5, 0x40

#define kMaskRandom10_3 \
  0xa4, 0x40, \
  0xc9, 0x00, \
  0x52, 0x80

#define kMaskRandom10_4 \
  0xca, 0x00, \
  0x32, 0x80, \
  0xa1, 0x40, \
  0x55, 0x00

#define kMaskRandom10_5 \
  0xca, 0x00, \
  0x32, 0x80, \
  0xa1, 0x40, \
  0x55, 0x00, \
  0x08, 0xc0

#define kMaskRandom10_6 \
  0x0e, 0x00, \
  0x33, 0x00, \
  0x10, 0xc0, \
  0x45, 0x40, \
  0x88, 0x80, \
  0xe0, 0x00

#define kMaskRandom10_7 \
  0x46, 0x00, \
  0x33, 0x00, \
  0x80, 0xc0, \
  0x0c, 0x40, \
  0x28, 0x80, \
  0x94, 0x00, \
  0xc1, 0x00

#define kMaskRandom10_8 \
  0x2c, 0x00, \
  0x81, 0x80, \
  0xa0, 0x40, \
  0x05, 0x40, \
  0x18, 0x80, \
  0xc2, 0x00, \
  0x22, 0x80, \
  0x50, 0x40

#define kMaskRandom10_9 \
  0x4c, 0x00, \
  0x23, 0x00, \
  0x88, 0xc0, \
  0x21, 0x40, \
  0x52, 0x80, \
  0x94, 0x00, \
  0x26, 0x00, \
  0x48, 0x40, \
  0x91, 0x80

#define kMaskRandom11_1 \
  0xff, 0xe0

#define kMaskRandom11_10 \
  0x64, 0x40, \
  0x51, 0x40, \
  0xa9, 0x00, \
  0x04, 0xc0, \
  0xd0, 0x00, \
  0x82, 0x40, \
  0x21, 0x20, \
  0x0c, 0x20, \
  0x4a, 0x00, \
  0x12, 0xa0

#define kMaskRandom11_11 \
  0x46, 0x40, \
  0x33, 0x20, \
  0x99, 0x00, \
  0x05, 0x80, \
  0x80, 0xa0, \
  0x84, 0x40, \
  0x40, 0x60, \
  0x0a, 0x80, \
  0x68, 0x00, \
  0x10, 0x20, \
  0x30, 0x40

#define kMaskRandom11_2 \
  0xec, 0xc0, \
  0x9b, 0xa0

#define kMaskRandom11_3 \
  0xca, 0xc0, \
  0xf1, 0x40, \
  0xb6, 0x20

#define kMaskRandom11_4 \
  0xc4, 0xc0, \
  0x31, 0x60, \
  0x4b, 0x20, \
  0x2c, 0xa0

#define kMaskRandom11_5 \
  0x86, 0x80, \
  0x23, 0x20, \
  0x16, 0x20, \
  0x4c, 0x20, \
  0x41, 0xc0

#define kMaskRandom11_6 \
  0x64, 0x40, \
  0x51, 0x40, \
  0x0c, 0xa0, \
  0xa1, 0x20, \
  0x12, 0xa0, \
  0x8a, 0x40

#define kMaskRandom11_7 \
  0x46, 0x40, \
  0x33, 0x20, \
  0x91, 0x80, \
  0xa4, 0x20, \
  0x50, 0xa0, \
  0x84, 0xc0, \
  0x09, 0x60

#define kMaskRandom11_8 \
  0x0c, 0x80, \
  0x80, 0x60, \
  0xa0, 0x80, \
  0x05, 0x40, \
  0x43, 0x00, \
  0x1a, 0x00, \
  0x60, 0x20, \
  0x14, 0x20

#define kMaskRandom11_9 \
  0x46, 0x40, \
  0x62, 0x60, \
  0x8c, 0x00, \
  0x01, 0x60, \
  0x07, 0x80, \
  0xa0, 0x80, \
  0x18, 0xa0, \
  0x91, 0x00, \
  0x78, 0x00

#define kMaskRandom12_1 \
  0xff, 0xf0

#define kMaskRandom12_10 \
  0x51, 0x40, \
  0x45, 0x10, \
  0x80, 0xd0, \
  0x24, 0x20, \
  0x0a, 0x20, \
  0x00, 0xe0, \
  0xb8, 0x00, \
  0x09, 0x10, \
  0x56, 0x00, \
  0xa2, 0x80

#define kMaskRandom12_11 \
  0x53, 0x60, \
  0x21, 0x30, \
  0x10, 0x90, \
  0x00, 0x70, \
  0x0c, 0x10, \
  0x40, 0xc0, \
  0x6a, 0x00, \
  0x86, 0x00, \
  0x24, 0x80, \
  0x89, 0x00, \
  0xc0, 0x20

#define kMaskRandom12_12 \
  0x10, 0x60, \
  0x02, 0x30, \
  0x40, 0x50, \
  0x21, 0x80, \
  0x81, 0x10, \
  0x14, 0x80, \
  0x98, 0x00, \
  0x08, 0x90, \
  0x62, 0x00, \
  0x24, 0x20, \
  0x8a, 0x00, \
  0x84, 0x40

#define kMaskRandom12_2 \
  0xec, 0xc0, \
  0x93, 0xb0

#define kMaskRandom12_3 \
  0x9b, 0x80, \
  0x4f, 0x10, \
  0x3c, 0x60

#define kMaskRandom12_4 \
  0x8b, 0x20, \
  0x14, 0xb0, \
  0x22, 0xd0, \
  0x45, 0x50

#define kMaskRandom12_5 \
  0x53, 0x60, \
  0x64, 0x20, \
  0x0c, 0xc0, \
  0x82, 0xa0, \
  0x09, 0x30

#define kMaskRandom12_6 \
  0x51, 0x40, \
  0xc5, 0x10, \
  0x21, 0x80, \
  0x12, 0x30, \
  0x08, 0xe0, \
  0x2e, 0x00

#define kMaskRandom12_7 \
  0x53, 0x60, \
  0x21, 0x30, \
  0x90, 0x90, \
  0x02, 0x50, \
  0x06, 0xa0, \
  0x2c, 0x00, \
  0x88, 0x60

#define kMaskRandom12_8 \
  0x20, 0x60, \
  0x80, 0x30, \
  0x42, 0x40, \
  0x01, 0x90, \
  0x14, 0x10, \
  0x0a, 0x80, \
  0x38, 0x00, \
  0xc5, 0x00

#define kMaskRandom12_9 \
  0x53, 0x60, \
  0xe4, 0x20, \
  0x24, 0x40, \
  0xa1, 0x10, \
  0x18, 0x30, \
  0x03, 0x90, \
  0x8a, 0x10, \
  0x04, 0x90, \
  0x00, 0xe0

#define kPacketMaskRandom1 1, \
  kMaskRandom1_1

#define kPacketMaskRandom2 2, \
  kMaskRandom2_1, \
  kMaskRandom2_2

#define kPacketMaskRandom3 3, \
  kMaskRandom3_1, \
  kMaskRandom3_2, \
  kMaskRandom3_3

#define kPacketMaskRandom4 4, \
  kMaskRandom4_1, \
  kMaskRandom4_2, \
  kMaskRandom4_3, \
  kMaskRandom4_4

#define kPacketMaskRandom5 5, \
  kMaskRandom5_1, \
  kMaskRandom5_2, \
  kMaskRandom5_3, \
  kMaskRandom5_4, \
  kMaskRandom5_5

#define kPacketMaskRandom6 6, \
  kMaskRandom6_1, \
  kMaskRandom6_2, \
  kMaskRandom6_3, \
  kMaskRandom6_4, \
  kMaskRandom6_5, \
  kMaskRandom6_6

#define kPacketMaskRandom7 7, \
  kMaskRandom7_1, \
  kMaskRandom7_2, \
  kMaskRandom7_3, \
  kMaskRandom7_4, \
  kMaskRandom7_5, \
  kMaskRandom7_6, \
  kMaskRandom7_7

#define kPacketMaskRandom8 8, \
  kMaskRandom8_1, \
  kMaskRandom8_2, \
  kMaskRandom8_3, \
  kMaskRandom8_4, \
  kMaskRandom8_5, \
  kMaskRandom8_6, \
  kMaskRandom8_7, \
  kMaskRandom8_8

#define kPacketMaskRandom9 9, \
  kMaskRandom9_1, \
  kMaskRandom9_2, \
  kMaskRandom9_3, \
  kMaskRandom9_4, \
  kMaskRandom9_5, \
  kMaskRandom9_6, \
  kMaskRandom9_7, \
  kMaskRandom9_8, \
  kMaskRandom9_9

#define kPacketMaskRandom10 10, \
  kMaskRandom10_1, \
  kMaskRandom10_2, \
  kMaskRandom10_3, \
  kMaskRandom10_4, \
  kMaskRandom10_5, \
  kMaskRandom10_6, \
  kMaskRandom10_7, \
  kMaskRandom10_8, \
  kMaskRandom10_9, \
  kMaskRandom10_10

#define kPacketMaskRandom11 11, \
  kMaskRandom11_1, \
  kMaskRandom11_2, \
  kMaskRandom11_3, \
  kMaskRandom11_4, \
  kMaskRandom11_5, \
  kMaskRandom11_6, \
  kMaskRandom11_7, \
  kMaskRandom11_8, \
  kMaskRandom11_9, \
  kMaskRandom11_10, \
  kMaskRandom11_11

#define kPacketMaskRandom12 12, \
  kMaskRandom12_1, \
  kMaskRandom12_2, \
  kMaskRandom12_3, \
  kMaskRandom12_4, \
  kMaskRandom12_5, \
  kMaskRandom12_6, \
  kMaskRandom12_7, \
  kMaskRandom12_8, \
  kMaskRandom12_9, \
  kMaskRandom12_10, \
  kMaskRandom12_11, \
  kMaskRandom12_12
// clang-format on
} // namespace

namespace fec_private_tables {

const uint8_t kPacketMaskBurstyTbl[] = {
    12,
    kPacketMaskBursty1,
    kPacketMaskBursty2,
    kPacketMaskBursty3,
    kPacketMaskBursty4,
    kPacketMaskBursty5,
    kPacketMaskBursty6,
    kPacketMaskBursty7,
    kPacketMaskBursty8,
    kPacketMaskBursty9,
    kPacketMaskBursty10,
    kPacketMaskBursty11,
    kPacketMaskBursty12,
};

const uint8_t kPacketMaskRandomTbl[] = {
    12,
    kPacketMaskRandom1, // 2 byte entries.
    kPacketMaskRandom2,
    kPacketMaskRandom3,
    kPacketMaskRandom4,
    kPacketMaskRandom5,
    kPacketMaskRandom6,
    kPacketMaskRandom7,
    kPacketMaskRandom8,
    kPacketMaskRandom9,
    kPacketMaskRandom10,
    kPacketMaskRandom11,
    kPacketMaskRandom12,
};

} // namespace fec_private_tables
namespace internal {

// Shifts a mask by number of columns (bits), and fits it to an output mask.
// The mask is a matrix where the rows are the FEC packets,
// and the columns are the source packets the FEC is applied to.
// Each row of the mask is represented by a number of mask bytes.
//
// \param[in]  num_mask_bytes     The number of mask bytes of output mask.
// \param[in]  num_sub_mask_bytes The number of mask bytes of input mask.
// \param[in]  num_column_shift   The number columns to be shifted, and
//                                the starting row for the output mask.
// \param[in]  end_row            The ending row for the output mask.
// \param[in]  sub_mask           A pointer to hold the input mask, of size
//                                [0, (end_row_fec - start_row_fec) *
//                                    num_sub_mask_bytes]
// \param[out] packet_mask        A pointer to hold the output mask, of size
//                                [0, x * num_mask_bytes],
//                                where x >= end_row_fec.
// TODO(marpan): This function is doing three things at the same time:
// shift within a byte, byte shift and resizing.
// Split up into subroutines.
void ShiftFitSubMask(int num_mask_bytes, int res_mask_bytes, int num_column_shift, int end_row,
                     const uint8_t *sub_mask, uint8_t *packet_mask) {
	// Number of bit shifts within a byte
	const int num_bit_shifts = (num_column_shift % 8);
	const int num_byte_shifts = num_column_shift >> 3;

	// Modify new mask with sub-mask21.

	// Loop over the remaining FEC packets.
	for (int i = num_column_shift; i < end_row; ++i) {
		// Byte index of new mask, for row i and column res_mask_bytes,
		// offset by the number of bytes shifts
		int pkt_mask_idx = i * num_mask_bytes + res_mask_bytes - 1 + num_byte_shifts;
		// Byte index of sub_mask, for row i and column res_mask_bytes
		int pkt_mask_idx2 = (i - num_column_shift) * res_mask_bytes + res_mask_bytes - 1;

		uint8_t shift_right_curr_byte = 0;
		uint8_t shift_left_prev_byte = 0;
		uint8_t comb_new_byte = 0;

		// Handle case of num_mask_bytes > res_mask_bytes:
		// For a given row, copy the rightmost "numBitShifts" bits
		// of the last byte of sub_mask into output mask.
		if (num_mask_bytes > res_mask_bytes) {
			shift_left_prev_byte = (sub_mask[pkt_mask_idx2] << (8 - num_bit_shifts));
			packet_mask[pkt_mask_idx + 1] = shift_left_prev_byte;
		}

		// For each row i (FEC packet), shift the bit-mask of the sub_mask.
		// Each row of the mask contains "resMaskBytes" of bytes.
		// We start from the last byte of the sub_mask and move to first one.
		for (int j = res_mask_bytes - 1; j > 0; j--) {
			// Shift current byte of sub21 to the right by "numBitShifts".
			shift_right_curr_byte = sub_mask[pkt_mask_idx2] >> num_bit_shifts;

			// Fill in shifted bits with bits from the previous (left) byte:
			// First shift the previous byte to the left by "8-numBitShifts".
			shift_left_prev_byte = (sub_mask[pkt_mask_idx2 - 1] << (8 - num_bit_shifts));

			// Then combine both shifted bytes into new mask byte.
			comb_new_byte = shift_right_curr_byte | shift_left_prev_byte;

			// Assign to new mask.
			packet_mask[pkt_mask_idx] = comb_new_byte;
			pkt_mask_idx--;
			pkt_mask_idx2--;
		}
		// For the first byte in the row (j=0 case).
		shift_right_curr_byte = sub_mask[pkt_mask_idx2] >> num_bit_shifts;
		packet_mask[pkt_mask_idx] = shift_right_curr_byte;
	}
}

// Fits an input mask (sub_mask) to an output mask.
// The mask is a matrix where the rows are the FEC packets,
// and the columns are the source packets the FEC is applied to.
// Each row of the mask is represented by a number of mask bytes.
//
// \param[in]  num_mask_bytes     The number of mask bytes of output mask.
// \param[in]  num_sub_mask_bytes The number of mask bytes of input mask.
// \param[in]  num_rows           The number of rows of the input mask.
// \param[in]  sub_mask           A pointer to hold the input mask, of size
//                                [0, num_rows * num_sub_mask_bytes]
// \param[out] packet_mask        A pointer to hold the output mask, of size
//                                [0, x * num_mask_bytes], where x >= num_rows.
void FitSubMask(int num_mask_bytes, int num_sub_mask_bytes, int num_rows, const uint8_t *sub_mask,
                uint8_t *packet_mask) {
	if (num_mask_bytes == num_sub_mask_bytes) {
		memcpy(packet_mask, sub_mask, num_rows * num_sub_mask_bytes);
	} else {
		for (int i = 0; i < num_rows; ++i) {
			int pkt_mask_idx = i * num_mask_bytes;
			int pkt_mask_idx2 = i * num_sub_mask_bytes;
			for (int j = 0; j < num_sub_mask_bytes; ++j) {
				packet_mask[pkt_mask_idx] = sub_mask[pkt_mask_idx2];
				pkt_mask_idx++;
				pkt_mask_idx2++;
			}
		}
	}
}

enum ProtectionMode {
	kModeNoOverlap,
	kModeOverlap,
	kModeBiasFirstPacket,
};
PacketMaskTable::PacketMaskTable(int num_media_packets, FecMaskType fec_mask_type)
    : table_(PickTable(fec_mask_type, num_media_packets)) {}

PacketMaskTable::~PacketMaskTable() = default;

rtc::ArrayView<const uint8_t> PacketMaskTable::LookUp(int num_media_packets, int num_fec_packets) {

	if (num_media_packets <= 12) {
		return LookUpInFecTable(table_, num_media_packets - 1, num_fec_packets - 1);
	}
	int mask_length = static_cast<int>(PacketMaskSize(static_cast<size_t>(num_media_packets)));

	// Generate FEC code mask for {num_media_packets(M), num_fec_packets(N)} (use
	// N FEC packets to protect M media packets) In the mask, each FEC packet
	// occupies one row, each bit / coloumn represent one media packet. E.g. Row
	// A, Col/Bit B is set to 1, means FEC packet A will have protection for media
	// packet B.

	// Loop through each fec packet.
	for (int row = 0; row < num_fec_packets; row++) {
		// Loop through each fec code in a row, one code has 8 bits.
		// Bit X will be set to 1 if media packet X shall be protected by current
		// FEC packet. In this implementation, the protection is interleaved, thus
		// media packet X will be protected by FEC packet (X % N)
		for (int col = 0; col < mask_length; col++) {
			fec_packet_mask_[row * mask_length + col] =
			    ((col * 8) % num_fec_packets == row && (col * 8) < num_media_packets ? 0x80
			                                                                         : 0x00) |
			    ((col * 8 + 1) % num_fec_packets == row && (col * 8 + 1) < num_media_packets
			         ? 0x40
			         : 0x00) |
			    ((col * 8 + 2) % num_fec_packets == row && (col * 8 + 2) < num_media_packets
			         ? 0x20
			         : 0x00) |
			    ((col * 8 + 3) % num_fec_packets == row && (col * 8 + 3) < num_media_packets
			         ? 0x10
			         : 0x00) |
			    ((col * 8 + 4) % num_fec_packets == row && (col * 8 + 4) < num_media_packets
			         ? 0x08
			         : 0x00) |
			    ((col * 8 + 5) % num_fec_packets == row && (col * 8 + 5) < num_media_packets
			         ? 0x04
			         : 0x00) |
			    ((col * 8 + 6) % num_fec_packets == row && (col * 8 + 6) < num_media_packets
			         ? 0x02
			         : 0x00) |
			    ((col * 8 + 7) % num_fec_packets == row && (col * 8 + 7) < num_media_packets
			         ? 0x01
			         : 0x00);
		}
	}
	return {&fec_packet_mask_[0], static_cast<size_t>(num_fec_packets * mask_length)};
}

// If `num_media_packets` is larger than the maximum allowed by `fec_mask_type`
// for the bursty type, or the random table is explicitly asked for, then the
// random type is selected. Otherwise the bursty table callback is returned.
const uint8_t *PacketMaskTable::PickTable(FecMaskType fec_mask_type, int num_media_packets) {

	if (fec_mask_type != kFecMaskRandom &&
	    num_media_packets <= static_cast<int>(fec_private_tables::kPacketMaskBurstyTbl[0])) {
		return &fec_private_tables::kPacketMaskBurstyTbl[0];
	}

	return &fec_private_tables::kPacketMaskRandomTbl[0];
}

// Remaining protection after important (first partition) packet protection
void RemainingPacketProtection(int num_media_packets, int num_fec_remaining,
                               int num_fec_for_imp_packets, int num_mask_bytes, ProtectionMode mode,
                               uint8_t *packet_mask, PacketMaskTable *mask_table) {
	if (mode == kModeNoOverlap) {
		// sub_mask21

		const int res_mask_bytes = PacketMaskSize(num_media_packets - num_fec_for_imp_packets);

		auto end_row = (num_fec_for_imp_packets + num_fec_remaining);
		rtc::ArrayView<const uint8_t> packet_mask_sub_21 =
		    mask_table->LookUp(num_media_packets - num_fec_for_imp_packets, num_fec_remaining);

		ShiftFitSubMask(num_mask_bytes, res_mask_bytes, num_fec_for_imp_packets, end_row,
		                &packet_mask_sub_21[0], packet_mask);

	} else if (mode == kModeOverlap || mode == kModeBiasFirstPacket) {
		// sub_mask22
		rtc::ArrayView<const uint8_t> packet_mask_sub_22 =
		    mask_table->LookUp(num_media_packets, num_fec_remaining);

		FitSubMask(num_mask_bytes, num_mask_bytes, num_fec_remaining, &packet_mask_sub_22[0],
		           &packet_mask[num_fec_for_imp_packets * num_mask_bytes]);

		if (mode == kModeBiasFirstPacket) {
			for (int i = 0; i < num_fec_remaining; ++i) {
				int pkt_mask_idx = i * num_mask_bytes;
				packet_mask[pkt_mask_idx] = packet_mask[pkt_mask_idx] | (1 << 7);
			}
		}
	}
}

// Protection for important (first partition) packets
void ImportantPacketProtection(int num_fec_for_imp_packets, int num_imp_packets, int num_mask_bytes,
                               uint8_t *packet_mask, PacketMaskTable *mask_table) {
	const int num_imp_mask_bytes = PacketMaskSize(num_imp_packets);

	// Get sub_mask1 from table
	rtc::ArrayView<const uint8_t> packet_mask_sub_1 =
	    mask_table->LookUp(num_imp_packets, num_fec_for_imp_packets);

	FitSubMask(num_mask_bytes, num_imp_mask_bytes, num_fec_for_imp_packets, &packet_mask_sub_1[0],
	           packet_mask);
}

// This function sets the protection allocation: i.e., how many FEC packets
// to use for num_imp (1st partition) packets, given the: number of media
// packets, number of FEC packets, and number of 1st partition packets.
int SetProtectionAllocation(int num_media_packets, int num_fec_packets, int num_imp_packets) {
	// TODO(marpan): test different cases for protection allocation:

	// Use at most (alloc_par * num_fec_packets) for important packets.
	float alloc_par = 0.5;
	int max_num_fec_for_imp = alloc_par * num_fec_packets;

	int num_fec_for_imp_packets =
	    (num_imp_packets < max_num_fec_for_imp) ? num_imp_packets : max_num_fec_for_imp;

	// Fall back to equal protection in this case
	if (num_fec_packets == 1 && (num_media_packets > 2 * num_imp_packets)) {
		num_fec_for_imp_packets = 0;
	}

	return num_fec_for_imp_packets;
}

// Modification for UEP: reuse the off-line tables for the packet masks.
// Note: these masks were designed for equal packet protection case,
// assuming random packet loss.

// Current version has 3 modes (options) to build UEP mask from existing ones.
// Various other combinations may be added in future versions.
// Longer-term, we may add another set of tables specifically for UEP cases.
// TODO(marpan): also consider modification of masks for bursty loss cases.

// Mask is characterized as (#packets_to_protect, #fec_for_protection).
// Protection factor defined as: (#fec_for_protection / #packets_to_protect).

// Let k=num_media_packets, n=total#packets, (n-k)=num_fec_packets,
// m=num_imp_packets.

// For ProtectionMode 0 and 1:
// one mask (sub_mask1) is used for 1st partition packets,
// the other mask (sub_mask21/22, for 0/1) is for the remaining FEC packets.

// In both mode 0 and 1, the packets of 1st partition (num_imp_packets) are
// treated equally important, and are afforded more protection than the
// residual partition packets.

// For num_imp_packets:
// sub_mask1 = (m, t): protection = t/(m), where t=F(k,n-k,m).
// t=F(k,n-k,m) is the number of packets used to protect first partition in
// sub_mask1. This is determined from the function SetProtectionAllocation().

// For the left-over protection:
// Mode 0: sub_mask21 = (k-m,n-k-t): protection = (n-k-t)/(k-m)
// mode 0 has no protection overlap between the two partitions.
// For mode 0, we would typically set t = min(m, n-k).

// Mode 1: sub_mask22 = (k, n-k-t), with protection (n-k-t)/(k)
// mode 1 has protection overlap between the two partitions (preferred).

// For ProtectionMode 2:
// This gives 1st packet of list (which is 1st packet of 1st partition) more
// protection. In mode 2, the equal protection mask (which is obtained from
// mode 1 for t=0) is modified (more "1s" added in 1st column of packet mask)
// to bias higher protection for the 1st source packet.

// Protection Mode 2 may be extended for a sort of sliding protection
// (i.e., vary the number/density of "1s" across columns) across packets.

void UnequalProtectionMask(int num_media_packets, int num_fec_packets, int num_imp_packets,
                           int num_mask_bytes, uint8_t *packet_mask, PacketMaskTable *mask_table) {
	// Set Protection type and allocation
	// TODO(marpan): test/update for best mode and some combinations thereof.

	ProtectionMode mode = kModeOverlap;
	int num_fec_for_imp_packets = 0;

	if (mode != kModeBiasFirstPacket) {
		num_fec_for_imp_packets =
		    SetProtectionAllocation(num_media_packets, num_fec_packets, num_imp_packets);
	}

	int num_fec_remaining = num_fec_packets - num_fec_for_imp_packets;
	// Done with setting protection type and allocation

	//
	// Generate sub_mask1
	//
	if (num_fec_for_imp_packets > 0) {
		ImportantPacketProtection(num_fec_for_imp_packets, num_imp_packets, num_mask_bytes,
		                          packet_mask, mask_table);
	}

	//
	// Generate sub_mask2
	//
	if (num_fec_remaining > 0) {
		RemainingPacketProtection(num_media_packets, num_fec_remaining, num_fec_for_imp_packets,
		                          num_mask_bytes, mode, packet_mask, mask_table);
	}
}

// This algorithm is tailored to look up data in the `kPacketMaskRandomTbl` and
// `kPacketMaskBurstyTbl` tables. These tables only cover fec code for up to 12
// media packets. Starting from 13 media packets, the fec code will be generated
// at runtime. The format of those arrays is that they're essentially a 3
// dimensional array with the following dimensions: * media packet
//   * Size for kPacketMaskRandomTbl: 12
//   * Size for kPacketMaskBurstyTbl: 12
// * fec index
//   * Size for both random and bursty table increases from 1 to number of rows.
//     (i.e. 1-48, or 1-12 respectively).
// * Fec data (what actually gets returned)
//   * Size for kPacketMaskRandomTbl: 2 bytes.
//     * For all entries: 2 * fec index (1 based)
//   * Size for kPacketMaskBurstyTbl: 2 bytes.
//     * For all entries: 2 * fec index (1 based)
rtc::ArrayView<const uint8_t> LookUpInFecTable(const uint8_t *table, int media_packet_index,
                                               int fec_index) {

	// Skip over the table size.
	const uint8_t *entry = &table[1];

	uint8_t entry_size_increment = 2; // 0-16 are 2 byte wide, then changes to 6.

	// Hop over un-interesting array entries.
	for (int i = 0; i < media_packet_index; ++i) {
		if (i == 16)
			entry_size_increment = 6;
		uint8_t count = entry[0];
		++entry; // skip over the count.
		for (int j = 0; j < count; ++j) {
			entry += entry_size_increment * (j + 1); // skip over the data.
		}
	}

	if (media_packet_index == 16)
		entry_size_increment = 6;

	++entry; // Skip over the size.

	// Find the appropriate data in the second dimension.

	// Find the specific data we're looking for.
	for (int i = 0; i < fec_index; ++i)
		entry += entry_size_increment * (i + 1); // skip over the data.

	size_t size = entry_size_increment * (fec_index + 1);
	return {&entry[0], size};
}

void GeneratePacketMasks(int num_media_packets, int num_fec_packets, PacketMaskTable *mask_table,
                         uint8_t *packet_mask) {
	bool use_unequal_protection = false;
	int num_imp_packets = 0;
	const int num_mask_bytes = PacketMaskSize(num_media_packets);

	// Equal-protection for these cases.
	if (!use_unequal_protection || num_imp_packets == 0) {
		// Retrieve corresponding mask table directly:for equal-protection case.
		// Mask = (k,n-k), with protection factor = (n-k)/k,
		// where k = num_media_packets, n=total#packets, (n-k)=num_fec_packets.
		rtc::ArrayView<const uint8_t> mask = mask_table->LookUp(num_media_packets, num_fec_packets);
		memcpy(packet_mask, &mask[0], mask.size());
	} else { // UEP case
		UnequalProtectionMask(num_media_packets, num_fec_packets, num_imp_packets, num_mask_bytes,
		                      packet_mask, mask_table);
	} // End of UEP modification
} // End of GetPacketMasks

size_t PacketMaskSize(size_t num_sequence_numbers) {
	if (num_sequence_numbers > 8 * kUlpfecPacketMaskSizeLBitClear) {
		return kUlpfecPacketMaskSizeLBitSet;
	}
	return kUlpfecPacketMaskSizeLBitClear;
}

void InsertZeroColumns(int num_zeros, uint8_t *new_mask, int new_mask_bytes, int num_fec_packets,
                       int new_bit_index) {
	for (uint16_t row = 0; row < num_fec_packets; ++row) {
		const int new_byte_index = row * new_mask_bytes + new_bit_index / 8;
		const int max_shifts = (7 - (new_bit_index % 8));
		new_mask[new_byte_index] <<= std::min(num_zeros, max_shifts);
	}
}

void CopyColumn(uint8_t *new_mask, int new_mask_bytes, uint8_t *old_mask, int old_mask_bytes,
                int num_fec_packets, int new_bit_index, int old_bit_index) {
	// Copy column from the old mask to the beginning of the new mask and shift it
	// out from the old mask.
	for (uint16_t row = 0; row < num_fec_packets; ++row) {
		int new_byte_index = row * new_mask_bytes + new_bit_index / 8;
		int old_byte_index = row * old_mask_bytes + old_bit_index / 8;
		new_mask[new_byte_index] |= ((old_mask[old_byte_index] & 0x80) >> 7);
		if (new_bit_index % 8 != 7) {
			new_mask[new_byte_index] <<= 1;
		}
		old_mask[old_byte_index] <<= 1;
	}
}

} // namespace internal