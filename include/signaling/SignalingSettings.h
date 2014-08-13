#ifndef WEBRTC_SIGNALING_SETTINGS_H
#define WEBRTC_SIGNALING_SETTINGS_H

#include <string>

namespace sig {

  class SignalingSettings {
  public:
    SignalingSettings();
    ~SignalingSettings();

  public:
    std::string port;
  };

} /* namespace sig */

#endif
