/*

  DecoderVP8
  ----------
  Decodes raw VP8 data. Used to decode the RTP payload data when it's
  encoded with VP8.

 */
#ifndef VIDEO_DECODER_VP8_H
#define VIDEO_DECODER_VP8_H

#define VPX_CODEC_DISABLE_COMPAT 1
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>
#include <stdint.h>

#define vpx_dx_interface (vpx_codec_vp8_dx())

namespace video {

  class DecoderVP8;
  typedef void(*decoder_vp8_on_image)(DecoderVP8* dec, const vpx_image_t* img);  /* gets called when the decoder decodes a new image */

  class DecoderVP8 {

  public:
    DecoderVP8();
    ~DecoderVP8();
    int init();
    int decode(uint8_t* data, size_t nbytes);

  public:
    vpx_codec_ctx_t ctx;
    vpx_image_t* img;
    bool is_init;

    /* callback */
    decoder_vp8_on_image on_image;
    void* user;
  }; 

} /* namespace video */

#endif
