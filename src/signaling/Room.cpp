#include <signaling/Room.h>

namespace sig {

  Room::Room(std::string name, std::string sdp)
    :sdp(sdp)
    ,name(name)
  {

  }

} /* namespace sig */
