#include <arpa/inet.h> // nthos
#include <stdio.h>
#include <stdlib.h>
#include <rtp/ReaderVP8.h>

namespace rtp {

  /* ----------------------------------------------------------- */

  int rtp_vp8_decode(uint8_t* data, uint32_t nbytes, PacketVP8* pkt) {

    uint8_t* buf = data;
    int64_t len = nbytes;

    if (!data) { return -1; } 
    if (!nbytes) { return -2; } 
    if (!pkt) { return -3; } 

    /* RTP Heoader */
    pkt->version         = (buf[0] & 0xC0) >> 6;
    pkt->padding         = (buf[0] & 0x20) >> 4;
    pkt->extension       = (buf[0] & 0x10) >> 3;
    pkt->csrc_count      = (buf[0] & 0x0F);
    pkt->marker          = (buf[1] & 0x80) >> 7;
    pkt->payload_type    = (buf[1] & 0x7F);
    pkt->sequence_number = ntohs(*(uint16_t*)(buf + 2));
    pkt->timestamp       = ntohl(*(uint32_t*)(buf + 4));
    pkt->ssrc            = ntohl(*(uint32_t*)(buf + 8));
    
    if (pkt->csrc_count != 0) {
      printf("ReaderVP::process - error: the csrc_count != 0, we only implemented support for csrc_count === 0.\n");
      return -4;
    }

    /* VP8-Payload-Descriptor */
    pkt->X     = (buf[12] & 0x80) >> 7;                                /* Extended control bits present */
    pkt->N     = (buf[12] & 0x20) >> 5;                                /* None reference frame. (if 1, we can discard this frame). */
    pkt->S     = (buf[12] & 0x10) >> 4;                                /* Start of VP8 partition */
    pkt->PID   = (buf[12] & 0x07);                                     /* Partition index */
    buf += 13;
    len -= 13;

    /*  X: |I|L|T|K| RSV  | (OPTIONAL)  */
    if(pkt->X == 1) {
      pkt->I = (buf[0] & 0x80) >> 7;                                        /* PictureID present */
      pkt->L = (buf[0] & 0x40) >> 6;                                        /* TL0PICIDX present */
      pkt->T = (buf[0] & 0x20) >> 5;                                        /* TID present */
      pkt->K = (buf[0] & 0x10) >> 4;                                        /* KEYIDX present */
      buf++;
      len--;
    }

    if(pkt->I) {
      pkt->M = (buf[0] & 0x80) >> 7;                                        /* M, PictureID extension flag. */

      if(pkt->M) {                                                          /* M, if M == 1, the picture ID takes 16 bits */
        pkt->PictureID = ntohs(*(uint16_t*)buf) & 0x7FFF;
        buf += 2;
        len -= 2;
      }
      else {
        buf++;
        len--;
      }
    }

    if (pkt->L) {
      buf++;
      len--;
    }

    if (pkt->T || pkt->K) {
      buf++;
      len--;
    }

    pkt->payload = buf;
    pkt->nbytes = len;

    printf("ReaderVP8::process - verbose: version: %d, "
                      "padding: %d, extension: %d, csrc_count: %d, "
                      "marker: %d, sequence: %u, timestamp: %u, ssrc: %u, payload_type: %u\n",
                       pkt->version, pkt->padding, pkt->extension, 
                       pkt->csrc_count, pkt->marker, pkt->sequence_number, 
                       pkt->timestamp, pkt->ssrc, pkt->payload_type);

    printf("ReaderVP8::process - verbose: X: %d, N: %d, S: %d, PID: %d, "
           "I: %d, L: %d, T: %d, K: %d, M:%d, PictureID: %u, len: %u\n",
           pkt->X, pkt->N, pkt->S, pkt->PID,
           pkt->I, pkt->L, pkt->T, pkt->K, pkt->M, pkt->PictureID, pkt->nbytes
           );

    return 0;
  }


} /* namespace rtp */
