#include <uv.h> /* @todo remove, tmp used to keep track of timestamp */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <rtp/WriterVP8.h>

namespace rtp {

  /* ------------------------------------------------------------------------ */

  static uint32_t rtp_vp8_calc_packet_size(uint32_t framelen, uint32_t mtu);

  /* ------------------------------------------------------------------------ */

  WriterVP8::WriterVP8() 
    :capacity(1024)
    ,buffer(NULL)
    ,on_packet(NULL)
    ,user(NULL)
  {

    /* allocate our buffer. */
    uint8_t* tmp = (uint8_t*)realloc(buffer, capacity);
    if (!tmp) {
      printf("WriterVP8 - error: cannot allocate buffer for RTP VP8 writer.\n");
      exit(1);
    }
    buffer = tmp;

    srand(time(NULL));
    picture_id = rand();
    seqnum = (rand() + 1) & 0x7FFFF;  /* not 0 */
    ssrc = (rand() + 1) & 0x7FFFF; /* not 0 */
  }

  WriterVP8::~WriterVP8() {
    if (buffer) {
      free(buffer);
    }
    buffer = NULL;
    capacity = 0;
  }

  int WriterVP8::packetize(const vpx_codec_cx_pkt_t* pkt) {
    /* @todo tmp: hardcoding timestamp to test RTP stream */
    static uint64_t start_time = 0;
    uint64_t ts = 0;
    if (0 == start_time) {
      start_time = uv_hrtime();
    }
    ts = ((uv_hrtime() - start_time) / (1000llu * 1000llu)) * 90;
    printf("TIMESTAMP: %llu\n", ts);
    /* @todo - end */
    

    if (!pkt) { return -1; } 
    if (!buffer) { return -2; } 
    if (!on_packet){ return -3; } 

    uint32_t mtu = 900 - 14; /* the header size for rtp/rtp-vp8 is roughly 14 bytes, @todo we need better heuristics here ^.^ */
    uint32_t packet_size = 0;
    uint32_t packet_dx = 0;   
    uint32_t bytes_left = 0;
    uint8_t* picid_ptr = (uint8_t*)&picture_id;
    uint8_t* tmp = NULL;
    PacketVP8 rtp;
    
    /* do we need to grow? */
    while (capacity < mtu) {
      capacity *= 2;
      tmp = (uint8_t*)realloc(buffer, capacity);
      if (NULL == tmp) {
        printf("WriterVP8 - error: cannot reallocate the buffer.\n");
      }
      buffer = tmp;
    }

    /* @todo the rtp.N (non-reference frame), should probably be set using a different flag, check out https://gist.github.com/roxlu/ceb1e8c95aff5ba60f45#file-vp8_impl-cc-L42 */
       

    /* update the given PacketVP8 so it fits the RFC */
    rtp.version = 2;                                                 /* RTP: version. */  
    rtp.extension = 0;                                               /* RTP: extension header? -> yes, VP8 */
    rtp.csrc_count = 0;                                              /* RTP: num of csrc identifiers */
    rtp.sequence_number = seqnum;                                    /* RTP: sequence number. */
    rtp.timestamp = pkt->data.frame.pts * 90;                        /* RTP: timestamp: 90hz. */
    rtp.ssrc = ssrc;                                                 /* RTP: ssrc */
    rtp.payload_type = 100;
    rtp.PID = pkt->data.frame.partition_id;                          /* RTP VP8: partition index. */
    rtp.S = 1;                                                       /* RTP VP8: start of first VP8 partition */
    rtp.X = 1;                                                       /* RTP VP8: extended control bits are present. */ 
    rtp.N = (pkt->data.frame.flags & VPX_FRAME_IS_KEY) ? 0 : 1;      /* RTP VP8: non-reference frame */
    rtp.M = 1;                                                       /* RTP VP8: we use 15 bits for the picture_id */
    rtp.PictureID = picture_id;                                      /* RTP VP8: picture id */
    rtp.payload = buffer;
   
    if (pkt->data.frame.flags & VPX_FRAME_IS_DROPPABLE) {
      exit(1);
    }

    /* create packets */
    bytes_left = pkt->data.frame.sz;
    while (bytes_left > 0) {
        
      /* calculate the size for this packet. */
      packet_size = rtp_vp8_calc_packet_size(bytes_left, mtu);
      bytes_left -= packet_size;

      /* set RTP packet properties */
      rtp.sequence_number = seqnum;
      rtp.marker = (packet_size < mtu) && (pkt->data.frame.flags & VPX_FRAME_IS_FRAGMENT) == 0;

      /* RTP header */
      buffer[0]  = (rtp.version      & 0x02) << 6;                   /* RTP: version */
      buffer[0] |= (rtp.padding      & 0x01) << 5;                   /* RTP: padding */ 
      buffer[0] |= (rtp.extension    & 0x01) << 4;                   /* RTP: has extension header */
      buffer[0] |= (rtp.csrc_count   & 0x0f) << 0;                   /* RTP: csrc count */
      buffer[1]  = (rtp.marker       & 0x01) << 7;                   /* RTP: marker bit, last packet of frame */ 
      buffer[1] |= (rtp.payload_type & 0x7f) << 0;                   /* RTP: payload type */ 
      *(uint16_t*)(buffer + 2) = htons(rtp.sequence_number);         /* RTP: sequence number */
      *(uint32_t*)(buffer + 4) = htonl(rtp.timestamp);               /* RTP: timestamp */ 
      *(uint32_t*)(buffer + 8) = htonl(rtp.ssrc);                    /* RTP: ssrc */

      /* RTP-VP8 required header */
      buffer[12]  = (rtp.X & 0x01)   << 7;                           /* RTP VP8: extended control bits set? */
      buffer[12] |= (rtp.N & 0x01)   << 5;                           /* RTP VP8: non-reference frame. */
      buffer[12] |= (rtp.S & 0x01)   << 4;                           /* RTP VP8: start of vp8-partition. */
      buffer[12] |= (rtp.PID & 0x07) << 0;                           /* RTP VP8: parition id. */
      
      /* RTP-VP8 extended control bits */
      buffer[13] = 0x80;                                             /* RTP VP8: picture id present, all other bits are 0. */
      buffer[14] = 0x80 | (picid_ptr[1]);                            /* RTP VP8: first bit sequence of picture id */     
      buffer[15] = picid_ptr[0];                                     /* RTP VP8: second bit sequence of picture id */

      /* copy the VP8 data */
      memcpy(buffer + 16, (uint8_t*)(pkt->data.frame.buf) + packet_dx, packet_size);
      packet_dx += packet_size;
      rtp.nbytes = 16 + packet_size;

      /* call the callback we have a new RTP packet. */
      on_packet(&rtp, user);

#if 1
      printf("WriterVP8::packtize - verbose: X: %d, N: %d, S: %d, PID: %d, payload_type: %d, SSRC: %u"
             "I: %d, L: %d, T: %d, K: %d, M:%d, PictureID: %u, len: %u, timestamp: %u\n",
             rtp.X, rtp.N, rtp.S, rtp.PID, rtp.payload_type, rtp.ssrc,
             rtp.I, rtp.L, rtp.T, rtp.K, rtp.M, rtp.PictureID, rtp.nbytes, rtp.timestamp
           );
#endif

      rtp.S = 0; /* for all other the 'start of partition is 0', except first packet the S = 0. */
      seqnum++;
    }

    picture_id = (picture_id + 1) & 0x7FFF;

    return 0;
  }

  /* ------------------------------------------------------------------------ */

  static uint32_t rtp_vp8_calc_packet_size(uint32_t framelen, uint32_t mtu) {
    if (framelen >= mtu) {
      return mtu;
    }
    else {
      return framelen;
    }
  }

} /* namespace rtp */
