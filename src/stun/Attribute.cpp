#include <stun/Attribute.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  Attribute::Attribute(uint16_t type)
    :type(type)
    ,length(0)
  {
  }


} /* namespace stun */
