#ifndef WEBRTC_SIGNALING_ROOM_H
#define WEBRTC_SIGNALING_ROOM_H

#include <string>

namespace sig {

  class Room {
  public:
    Room(std::string name, std::string sdp);

  public:
    std::string name;
    std::string sdp;
  };


} /* namespace sig */

#endif
