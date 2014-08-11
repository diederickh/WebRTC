/* 

   RTP VP8 extension (writer) 
   --------------------------

*/

#include <stdint.h>
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>
#include <rtp/PacketVP8.h>
#include <vector>

namespace rtp {
  
  typedef void(*rtp_vp8_on_packet)(PacketVP8* pkt, void* user);       /* gets called whenever a new RTP-VP8 packet is created; one vpx_codec_cx_pkt_t can result in multiple RTP-VP8 packets. */

  class WriterVP8 {

  public:
    WriterVP8();
    ~WriterVP8();
    int packetize(const vpx_codec_cx_pkt_t* pkt);                     /* create a RTP-VP8 packet. */

  public:
    uint32_t ssrc;                                                    /* RTP ssrc */
    uint16_t seqnum;                                                  /* RTP sequence number, starts with a random value. */
    uint16_t picture_id;                                              /* RTP-VP8 picture id, starts with a random value. */
    rtp_vp8_on_packet on_packet;                                      /* must be set by user; will receive a RTP packet. */
    void* user;                                                       /* gets passed into the callback */

  private:
    uint32_t capacity;                                                /* the capacity of our buffer */
    uint8_t* buffer;                                                  /* the buffer that will hold the VP8 data. */
  };


} /* namespace rtp */
