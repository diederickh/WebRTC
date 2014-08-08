#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <video/AggregatorVP8.h>

namespace video {

  AggregatorVP8::AggregatorVP8(uint32_t capacity) 
    :capacity(capacity)
    ,pos(0)
    ,nbytes(0)
    ,prev_seqnum(0)
    ,buffer(NULL)
  {
    if (!capacity) {
      printf("AggregatorVP8 - error: invalid capacity.\n");
      ::exit(1);
    }

    buffer = new uint8_t[capacity];
  }

  AggregatorVP8::~AggregatorVP8() {
    if (buffer) {
      delete[] buffer;
      buffer = NULL;
    }

    pos = 0;
    prev_seqnum = 0;
    nbytes = 0;
  }

  int AggregatorVP8::addPacket(rtp::PacketVP8* pkt) {

    uint64_t new_pos = 0;

    /* validate packet. */
    if (NULL == pkt) {
      return -1;
    }

    if (0 == pkt->nbytes || NULL == pkt->payload) {
      return -1;
    }
    
    /* are the sequence number incrementing monotonically */
    if (prev_seqnum != 0 && prev_seqnum != (pkt->sequence_number - 1)) {
      pos = 0;
      prev_seqnum = pkt->sequence_number;
      return -1;
    }

    /* check for buffer overflow */
    new_pos = pos + pkt->nbytes;
    if (new_pos >= capacity) {
      printf("AggregatorVP8 - error: cannot write because the frame is too large for our buffer.\n");
      return -1;
    }

    /* copy the data */
    memcpy(buffer + pos, pkt->payload, pkt->nbytes);

    prev_seqnum = pkt->sequence_number;
    pos += pkt->nbytes;

    if (1 == pkt->marker) {
      printf("----------------------------------- GOT A COMPLETE FRAME!!! ------------------------- \n");
      nbytes = pos;
      pos = 0;
      return 1;
    }

    return 0;
  }

} /* namespace video */
