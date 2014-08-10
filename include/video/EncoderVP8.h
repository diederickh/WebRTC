/*

  EncoderVP8
  -----------
  
  Basic implementation of the VP8 encoder. Used to encode YUV420P data that 
  one can stream to a WebRTC capable agent.

  Todo
  ----
  
  Optimize for low latency:
      - https://groups.google.com/a/webmproject.org/forum/#!topic/webm-discuss/TNPgd7Tf9jQ

 */

#ifndef VIDEO_ENCODER_VP8_H
#define VIDEO_ENCODER_VP8_H

#include <video/EncoderSettings.h>
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>
#define vpx_cx_interface (vpx_codec_vp8_cx())

namespace video {
  
  class EncoderVP8;
  typedef void(*encoder_vp8_on_packet)(EncoderVP8* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts);  /* gets called whenever we output a partition/packet */

  class EncoderVP8 {

  public:
    EncoderVP8();
    ~EncoderVP8();
    int init(EncoderSettings config);
    int encode(uint8_t* y, int ystride, uint8_t* u, int ustride, uint8_t* v, int vstride, int64_t pts);
    int encode(vpx_image_t* image, int64_t pts);

  public:
    EncoderSettings settings;
    vpx_image_t img;
    vpx_codec_enc_cfg_t cfg;
    vpx_codec_ctx_t ctx;
    unsigned long frame_duration;
    uint64_t nframes;
    uint32_t flags;                     /* Used to e.g. force keyframes */
    
    /* callback */
    encoder_vp8_on_packet on_packet;
    void* user;
  };

} /* namespace video */

#endif
