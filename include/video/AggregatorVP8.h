/*
  
  AggregatorVP8
  -------------

  Used to reconstruct VP8 frames from multiple RTP-VP8 packets. You 
  call addPacket() with a initialize PacketVP8. We collect data from the 
  same frame, from multiple packets into one buffer that can be fed into 
  the VP8 decoder. 

  * this is experimental code *

 */
#ifndef VIDEO_AGGREGATOR_VP8_H
#define VIDEO_AGGREGATOR_VP8_H

#include <rtp/PacketVP8.h>
#include <string>
#include <stdint.h>

enum {
  AGGREGATOR_VP8_ERR_PACKET = -1,
  AGGREGATOR_VP8_ERR_PAYLOAD = -2,
  AGGREGATOR_VP8_ERR_SEQNUM = -3,
  AGGREGATOR_VP8_ERR_BUFLEN = -4,
  AGGREGATOR_VP8_WANTS_MORE = 1,
  AGGREGATOR_VP8_GOT_FRAME = 2
};

namespace video {

  std::string aggregator_vp8_result_to_string(int r);

  class AggregatorVP8 {
  public:
    AggregatorVP8(uint32_t capacity = (1024 * 1024 *2));
    ~AggregatorVP8();
    int addPacket(rtp::PacketVP8* pkt);

  public:
    uint32_t capacity;                                     /* we collect data from multiple PacketVP8 into our 'buffer' this is the max size */
    uint8_t* buffer;                                       /* buffer that contains the frame data */
    uint32_t pos;                                          /* current write position */
    uint32_t nbytes;                                       /* number of bytes currently written to the buffer. */
    uint16_t prev_seqnum;                                  /* the previous sequence number that we handled. */
  };

} /* namespace video */

#endif
