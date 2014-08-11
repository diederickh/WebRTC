/*

  RTP VP8 extension (writer)
  --------------------------

  See:
  - RFC: http://tools.ietf.org/html/draft-ietf-payload-vp8-11#section-4.2
  - Example code: https://gist.github.com/roxlu/df0a786a8bf81e75ef0e

 */
#ifndef RTP_READER_VP8 
#define RTP_READER_VP8

#include <stdint.h>
#include <rtp/PacketVP8.h>

namespace rtp {

  int rtp_vp8_decode(uint8_t* data, uint32_t nbytes, PacketVP8* pkt);

} /* namespace rtp */

#endif
