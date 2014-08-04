#ifndef ICE_H
#define ICE_H

#include <string>
#include <vector>
#include <stun/Message.h>
#include <stun/Writer.h>

namespace ice {

  typedef void(*ice_on_data_callback)(uint8_t* data, uint32_t nbytes, void* user);  /* is called with data that needs to be send to the I/O that is using the ICE features */

  class ICE {
  public:
    ICE();
    void handleMessage(stun::Message* msg);            /* handles an STUN message you received from the other agent. */

  private:
    void handleBindingRequest(stun::Message* msg);

  public:
    std::string password; /* temp: is currently used to create a message integrity attribute and compute message integ.  */
    uint16_t port; /* temp: the port we use when adding the xor mapped address. */
    ice_on_data_callback on_data;
    void* user;
  };

} /* namespace ice */
#endif
