/*

  RTP VP8 extension
  -----------------

  See http://tools.ietf.org/html/draft-ietf-payload-vp8-11#section-4.2

 */
#ifndef RTP_READER_VP8 
#define RTP_READER_VP8

#include <stdint.h>
#include <rtp/PacketVP8.h>

namespace rtp {

  int rtp_vp8_decode(uint8_t* data, uint32_t nbytes, PacketVP8* pkt);

} /* namespace rtp */

#endif
