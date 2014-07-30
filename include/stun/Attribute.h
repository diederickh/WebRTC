#ifndef STUN_ATTRIBUTE_H
#define STUN_ATTRIBUTE_H

#include <stdint.h>
#include <string>
#include <vector>
#include <stun/Types.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  class Attribute {
  public:
    Attribute(uint16_t type = STUN_ATTR_TYPE_NONE);
    
  public:
    uint16_t type;
    uint16_t length;
  };

  /* --------------------------------------------------------------------- */

  class StringValue {
  public:
    StringValue(){}
    StringValue(std::string v) { std::copy(v.begin(), v.end(), std::back_inserter(buffer)); } 
    std::vector<uint8_t> buffer;
  };

  /* --------------------------------------------------------------------- */

  class Username : public Attribute {
  public:
    Username():Attribute(STUN_ATTR_USERNAME){ }
    Username(std::string name):value(name),Attribute(STUN_ATTR_USERNAME) { }
    StringValue value;
  };

} /* namespace stun */

#endif
