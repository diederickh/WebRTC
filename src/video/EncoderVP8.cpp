#include <stdio.h>
#include <video/EncoderVP8.h>

namespace video {

  EncoderVP8::EncoderVP8() 
    :nframes(0)
    ,flags(0)
    ,on_packet(NULL)
    ,user(NULL)
  {
  }

  EncoderVP8::~EncoderVP8() {
  }
  

  int EncoderVP8::init(EncoderSettings config) {
    vpx_codec_err_t err;
    settings = config;

    /* validate */
    if (!settings.width) { return -1; } 
    if (!settings.height) { return -2; } 
    
    /* initialize the config */
    /* @todo - set end usage */
    err = vpx_codec_enc_config_default(vpx_cx_interface, &cfg, 0);
    if (err) {
      printf("EncoderVP8 - error: cannot create vp8 codec config.\n");
      return -3;
    }

    /* update config */
    cfg.rc_target_bitrate = 1500;
    cfg.g_w = settings.width;
    cfg.g_h = settings.height;
    cfg.g_timebase.num = 1; 
    cfg.g_timebase.den = 1000;

#if 0
    cfg.g_pass = VPX_RC_ONE_PASS;
    cfg.g_error_resilient = 1;
    cfg.kf_mode = VPX_KF_AUTO;
    cfg.g_lag_in_frames = 0;
    cfg.rc_dropframe_thresh = 1;
    cfg.rc_end_usage = VPX_CBR;
#endif

    err = vpx_codec_enc_init(&ctx, vpx_cx_interface, &cfg, VPX_CODEC_CAP_OUTPUT_PARTITION);
    if (err) {
      printf("EncoderVP8 - error: cannot init the VP8 encoder.\n");
      return -4;
    }

    frame_duration = ((double) 1.0 / settings.fps_den) / ((double) cfg.g_timebase.num / cfg.g_timebase.den);
    flags = 0;
    nframes = 0;

    return 0;
  }

  //  int EncoderVP8::encode(unsigned char* pixels, uint32_t nbytse, int64_t pts) {
  int EncoderVP8::encode(uint8_t* y, int ystride, 
                         uint8_t* u, int ustride, 
                         uint8_t* v, int vstride, 
                         int64_t pts) 
  {

    if (!y) { return -1; } 
    if (!u) { return -2; } 
    if (!v) { return -3; } 

    img.w              = settings.width;
    img.h              = settings.height;
    img.d_w            = settings.width;
    img.d_h            = settings.height;
    img.fmt            = VPX_IMG_FMT_I420;
    img.planes[0]      = y;
    img.stride[0]      = ystride;
    img.planes[1]      = u;
    img.stride[1]      = ustride;
    img.planes[2]      = v;
    img.stride[2]      = vstride;
    img.x_chroma_shift = 1;
    img.y_chroma_shift = 1;
    img.bps            = 12;

#if 0
    printf("img.stride[0]: %d\n", img.stride[0]);
    printf("img.stride[1]: %d\n", img.stride[1]);
    printf("img.stride[2]: %d\n", img.stride[2]);
    printf("-\n");
#endif
  
    return encode(&img, pts);
  }

  int EncoderVP8::encode(vpx_image_t* image, int64_t pts) {

    int r = 0;
    vpx_codec_err_t err;
    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t* pkt;

    if (!on_packet) {
      printf("EncoderVP8 - warning: no on_packet handler set; not encoding.\n");
      return -1;
    }

    /* encode the packet. */
    err = vpx_codec_encode(&ctx, image, pts, frame_duration, flags, VPX_DL_REALTIME);
    if (err) {
      printf("EncoderVP8 - error: cannot encode.\n");
      return -2;
    }

    /* extract all the partitions. */
    while ( (pkt = vpx_codec_get_cx_data(&ctx, &iter)) ) {
      if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
        on_packet(this, pkt, pts);
      }
    }

    flags = 0;
    nframes++;

    return 0;
  }


} /* namespace video */
