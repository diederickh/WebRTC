#include <rtp/PacketVP8.h>

namespace rtp {

  PacketVP8::PacketVP8() {
    reset();
  }

  PacketVP8::~PacketVP8() {
    reset();
  }

  void PacketVP8::reset() {
    /* rtp header */
    version = 0; 
    padding = 0;
    extension = 0;
    csrc_count = 0;
    marker = 0;
    payload_type = 0;
    sequence_number = 0;
    timestamp = 0;

    /* vp8 */
    X = 0;
    N = 0;
    S = 0;
    PID = 0;
    
    I = 0;
    L = 0;
    T = 0;
    K = 0;
    PictureID = 0;
    TL0PICIDX = 0;
    M = 0;
    P = 0;
  }

} /* namespace rtp */
