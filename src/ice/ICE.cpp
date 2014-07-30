#include <ice/ICE.h>

namespace ice {

  ICE::ICE() {
  }

  /*
  void ICE::addCandidate(std::string c) {
    candidates.push_back(c);
  }
  */

  void ICE::handleMessage(stun::Message* msg) {

    if (!msg) { 
      return ; 
    } 

    switch (msg->type) {
      case stun::STUN_BINDING_REQUEST: {
        handleBindingRequest(msg);
        break;
      }
      default: {
        printf("Warning: unhandled STUN message in ICE: %s\n", stun::message_type_to_string(msg->type).c_str());
        break;
      }
    }
  }

  void ICE::handleBindingRequest(stun::Message* msg) {
    stun::Message resp(stun::STUN_BINDING_RESPONSE);
  }

} /* namespace ice */
