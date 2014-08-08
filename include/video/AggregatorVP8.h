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
#include <stdint.h>

namespace video {

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
