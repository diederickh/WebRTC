#include <video/EncoderSettings.h>

namespace video {
  
  EncoderSettings::EncoderSettings() {
    reset();
  }

  EncoderSettings::~EncoderSettings() {
    reset();
  }

  void EncoderSettings::reset() {
    width = 0;
    height = 0;
    fps_num = 0;
    fps_den = 0;
  }
  
} 
