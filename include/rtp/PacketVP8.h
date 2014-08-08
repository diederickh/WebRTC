#ifndef RTP_PACKET_VP8_H
#define RTP_PACKET_VP8_H

#include <stdint.h>

namespace rtp {

  class PacketVP8 {
  public:
    PacketVP8();
    ~PacketVP8();
    void reset();

  public:

    /* rtp header */
    uint8_t version;
    uint8_t padding;
    uint8_t extension;
    uint8_t csrc_count;
    uint8_t marker;                                     /* set for the very last packet of each encoded frame in line with the normal use of the M bit in video formats. For VP8 this will be set to 1 when the last packet for a frame is received. */
    uint8_t payload_type;
    uint16_t sequence_number;
    uint32_t timestamp;
    uint32_t ssrc;

    /* required */
    uint8_t X;                                          /* extended controlbits present */
    uint8_t N;                                          /* (non-reference frame)  when set to 1 this frame can be discarded */
    uint8_t S;                                          /* start of VP8 partition */
    uint8_t PID;                                        /* partition index */

    /* 2nd second row Payload Descriptor (is optional) */
    uint8_t I;                                          /* 1 if PictureID is present */
    uint8_t L;                                          /* 1 if TL0PICIDX is present */
    uint8_t T;                                          /* 1 if TID is present */ 
    uint8_t K;                                          /* 1 if KEYIDX is present */
    uint16_t PictureID;                                 /* 8 or 16 bits, picture ID */
    uint8_t TL0PICIDX;                                  /* 8 bits temporal level zero index */

    /* 3rd row Payload Descriptor */
    uint8_t M;                                          /* Extension flag; must be present if I bit == 1. If set, the PictureID field must contains 16 bits, else 8*/

    /* payload header */
    uint8_t P;                                          /* 0 if current frame is a key frame, otherwise 1 */

    /* the actual frame/partition data */
    uint8_t* payload;                                   /* points to the start of the partition data that can be fed into the decoder (once a frame has been constructed.). We do not copy the data! */
    uint32_t nbytes;                                    /* number of bytes in the partition */
  };

} /* namespace rtp */

#endif
