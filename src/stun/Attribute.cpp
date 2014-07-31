#include <string.h>
#include <stun/Attribute.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  Attribute::Attribute(uint16_t type)
    :type(type)
    ,length(0)
    ,nbytes(0)
  {
  }

  /* --------------------------------------------------------------------- */

  XorMappedAddress::XorMappedAddress()
    :Attribute(STUN_ATTR_XOR_MAPPED_ADDRESS)
    ,family(0)
    ,port(0)
  {
  }
  
  /* --------------------------------------------------------------------- */

  MessageIntegrity::MessageIntegrity() 
    :Attribute(STUN_ATTR_MESSAGE_INTEGRITY)
    ,offset(0)
  {
    memset(sha1, 0x00, 20);
  }

  /* --------------------------------------------------------------------- */
  


} /* namespace stun */
