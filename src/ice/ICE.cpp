#include <ice/ICE.h>

namespace ice {

  ICE::ICE() 
    :on_data(NULL)
    ,user(NULL)
  {
  }


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
    resp.addAttribute(new stun::Username("diederick"));
    resp.copyTransactionID(msg);

    stun::Writer writer;
    writer.writeMessage(&resp);
    
    if (on_data) {
      on_data(&writer.buffer[0], writer.buffer.size(), user);
    }
  }

} /* namespace ice */
