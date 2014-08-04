#include <stdlib.h>
#include <ice/ICE.h>

namespace ice {

  ICE::ICE() 
    :on_data(NULL)
    ,user(NULL)
    ,port(59976) /* temp */
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
   
    /*
    stun::XorMappedAddress* mapped_addr;
    if (msg->find(&mapped_addr)) {
      printf("Found a XorMappedAddress.\n");
      ::exit(0);
    }
    else {
      printf("We did not find a XorMappedAddress.\n");
      ::exit(0);
    }
    */

#if 0
    /* STUN BINDING REQUEST */
    {  
      stun::Message resp(stun::STUN_BINDING_REQUEST);
      resp.copyTransactionID(msg);
      resp.addAttribute(new stun::MessageIntegrity());
      resp.addAttribute(new stun::Fingerprint());

      stun::Writer writer;
      writer.writeMessage(&resp, password);
    
      if (on_data) {
        on_data(&writer.buffer[0], writer.buffer.size(), user);
      }
    }
#endif
    
    /* STUN BINDING RESPONSE */
    {
      stun::Message resp(stun::STUN_BINDING_RESPONSE);
      resp.copyTransactionID(msg);
      resp.addAttribute(new stun::XorMappedAddress("192.168.0.194", port));
      resp.addAttribute(new stun::MessageIntegrity());
      resp.addAttribute(new stun::Fingerprint());

      stun::Writer writer;
      writer.writeMessage(&resp, password);
      printf("Writer.buffer.size() == %lu\n", writer.buffer.size());
      //writer.writeMessage(&resp);
    
      if (on_data) {
        on_data(&writer.buffer[0], writer.buffer.size(), user);
      }
    }
  }

} /* namespace ice */
