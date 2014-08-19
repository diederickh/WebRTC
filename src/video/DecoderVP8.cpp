#include <stdio.h>
#include <vpx/vpx_encoder.h>
#include <video/DecoderVP8.h>

namespace video {

  DecoderVP8::DecoderVP8() 
    :img(NULL)
    ,is_init(false)
    ,on_image(NULL)
    ,user(NULL)
  {

  }

  DecoderVP8::~DecoderVP8() {
    if (true == is_init) {
      if (vpx_codec_destroy(&ctx)) {
        printf("DecoderVP8::~DecoderVP8 - error: cannot destroy vpx decoder.\n");
      }
    }
    is_init = false;
    img = NULL;
  }


  int DecoderVP8::init() {
    vpx_codec_err_t err;

    if (true == is_init) {
      printf("DecoderVP8::init - error: we're already initialized.\n");
      return -1;
    }
  
    err = vpx_codec_dec_init(&ctx, vpx_dx_interface, NULL, 0);
    if (err) {
      printf("DecoderVP8::init - error: cannot initialize the decoder: %s\n", vpx_codec_err_to_string(err));
      return -2;
    }
  
    is_init = true;

    return 0;
  }

  int DecoderVP8::decode(uint8_t* data, size_t nbytes) {

    vpx_codec_iter_t iter = NULL;
    vpx_codec_err_t err;
    img = NULL;

    if (false == is_init) {
      printf("DecoderVP8::decode - error: cannot decode, we're not yet initialized.\n");
      return -1;
    }

    if (NULL == on_image) {
      printf("DecoderVP8::decode - error: asking to decode but you haven't set an on_image handler.\n");
      return -2;
    }

    //  err = vpx_codec_decode(&ctx, (uint8_t*)pkt["data"].c_str(), pkt["data"].size(), NULL, 0) ; // VPX_DL_REALTIME);
    err = vpx_codec_decode(&ctx, data, nbytes, NULL, 0) ; // VPX_DL_REALTIME);
    if (err) {
      printf("DecoderVP8::decode - error: cannot decode a packet: %s\n", vpx_codec_err_to_string(err));
      return -3;
    }

    while ( (img = vpx_codec_get_frame(&ctx, &iter)) ) {
      on_image(this, img);
    }

    return 0;
  }

} /* namespace video */
