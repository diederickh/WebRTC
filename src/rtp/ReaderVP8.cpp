#include <arpa/inet.h> // nthos
#include <stdio.h>
#include <stdlib.h>
#include <rtp/ReaderVP8.h>

namespace rtp {

  /* ----------------------------------------------------------- */

  ReaderVP8::ReaderVP8() {
    reset();
  }

  ReaderVP8::~ReaderVP8() {
    reset();
  }

  int ReaderVP8::process(uint8_t* data, uint32_t nbytes) {

    uint8_t* buf = data;
    int64_t len = nbytes;

    if (!data) { return -1; } 
    if (!nbytes) { return -2; } 

    reset();

    /* RTP Header */
    version = (buf[0] & 0xC0) >> 6;
    padding = (buf[0] & 0x20) >> 4;
    extension = (buf[0] & 0x10) >> 3;
    csrc_count = (buf[0] & 0x0F);
    marker = (buf[1] & 0x80) >> 7;
    payload_type = (buf[1] & 0x7F);
    sequence_number = ntohs(*(uint16_t*)(buf+2));
    timestamp = ntohl(*(uint32_t*)(buf + 4));
    ssrc = ntohl(*(uint32_t*)(buf + 8));
    
    if (csrc_count != 0) {
      printf("ReaderVP::process - error: the csrc_count != 0, we only implemented support for csrc_count === 0.\n");
      return -3;
    }

    /* VP8-Payload-Descriptor */
    X = (buf[12] & 0x80) >> 7;                                  /* Extended control bits present */
    N = (buf[12] & 0x20) >> 5;                                  /* None reference frame. (if 1, we can discard this frame). */
    S = (buf[12] & 0x10) >> 4;                                  /* Start of VP8 partition */
    PID = (buf[12] & 0x07);                                     /* Partition index */
    buf = buf + 13;
    len -= 13;
    

    /*  X: |I|L|T|K| RSV  | (OPTIONAL)  */
    if(X == 1) {
      I = (buf[0] & 0x80) >> 7;                                 /* PictureID present */
      L = (buf[0] & 0x40) >> 6;                                 /* TL0PICIDX present */
      T = (buf[0] & 0x20) >> 5;                                 /* TID present */
      K = (buf[0] & 0x10) >> 4;                                 /* KEYIDX present */
      buf++;
      len--;
    }


    if(I) {
      M = (buf[0] & 0x80) >> 7;                                 /* M, PictureID extension flag. */
      PictureID = buf[0] & 0x7F;                                /* PictureID (only 7 bits) */
      buf++;
      len--

      if(buf[0] & 0x80) {                                       /* M, if M == 1, the picture ID takes 16 bits */
        PictureID = (PictureID << 8) +buf[1];                   /* PicureID (15 bits) */
        buf++;
        len--;
      }
    }

    printf("ReaderVP8::process - verbose: version: %d, "
                      "padding: %d, extension: %d, csrc_count: %d, "
                      "marker: %d, sequence: %u, timestamp: %u, ssrc: %u\n",
                       version, padding, extension, 
                       csrc_count, marker, sequence_number, 
                       timestamp, ssrc);

    printf("ReaderVP8::process - verbose: X: %d, N: %d, S: %d, PID: %d, "
           ", I: %d, L: %d, T: %d, K: %d, M:%d, PictureID: %u\n",
           X, N, S, PID,
           I, L, T, K, M, PictureID
           );

    return 0;
  }

  void ReaderVP8::reset() {

    /* rtp header */
    version = 0; 
    padding = 0;
    extension = 0;
    csrc_count = 0;
    marker = 0;
    payload_type = 0;
    sequence_number = 0;
    timestamp = 0;

    /* vp8 */
    X = 0;
    N = 0;
    S = 0;
    PID = 0;
    
    I = 0;
    L = 0;
    T = 0;
    K = 0;
    PictureID = 0;
    TL0PICIDX = 0;
    M = 0;
    P = 0;
  }


} /* namespace rtp */
