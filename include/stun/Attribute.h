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
    uint16_t length;    /* The number of bytes of the attribute data. This is the size w/o the padded bytes that are added when the the attribute is not padded to 32 bits. */
    uint16_t nbytes;    /* The number of bytes the attribute takes inside the buffer. Because the STUN message has to be padded on 32 bits the length may be different from the nbytes. Also, this nbytes includes the bytes of the type field and length field. */
    uint32_t offset;    /* Byte offset where the header of the attribute starts in the Message::buffer. */
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

  /* --------------------------------------------------------------------- */

  class Software : public Attribute {
  public:
    Software():Attribute(STUN_ATTR_SOFTWARE) {}
    Software(std::string name):value(name),Attribute(STUN_ATTR_SOFTWARE) {}
    StringValue value;
  };

  /* --------------------------------------------------------------------- */

  class XorMappedAddress : public Attribute {
  public:
    XorMappedAddress();
    XorMappedAddress(std::string addr, uint16_t p, uint8_t fam = STUN_IP4);
    uint8_t family;
    uint16_t port;
    std::string address;  /* IP address in string notation: 192.168.0.1 */
  };

  /* --------------------------------------------------------------------- */

  class Fingerprint : public Attribute {
  public:
    Fingerprint();
    uint32_t crc;
  };

  /* --------------------------------------------------------------------- */

  class IceControlled : public Attribute {
  public:
    IceControlled();
    uint64_t tie_breaker;
  };

  /* --------------------------------------------------------------------- */

  class IceControlling : public Attribute {
  public:
    IceControlling();
    uint64_t tie_breaker;
  };

  /* --------------------------------------------------------------------- */

  class Priority : public Attribute {
  public:
    Priority();
    uint32_t value;
  };

  /* --------------------------------------------------------------------- */

  class MessageIntegrity : public Attribute {
  public:
    MessageIntegrity();
    uint8_t sha1[20];
  };

} /* namespace stun */

#endif
