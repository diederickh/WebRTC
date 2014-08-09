#ifndef VIDEO_ENCODER_SETTINGS_H
#define VIDEO_ENCODER_SETTINGS_H

namespace video {
  
  class EncoderSettings {

  public:
    EncoderSettings();
    ~EncoderSettings();
    void reset();

  public:
    int width;
    int height;
    int fps_num;
    int fps_den;
  };

} /* namespace video */

#endif
