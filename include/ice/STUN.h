/*

  STUN
  ----

  Experimental code that parses stun. 

  References:
  ----------
  - libjingle: https://gist.github.com/roxlu/511544ea07610f70a900
  - own test code: https://gist.github.com/roxlu/57fe6c590f6267e0a209
  - C++, each attribute has its own class: https://github.com/husman/Development-Sample-Side-Projects/blob/713b052afd2344e95b7e1b33405713460d1b2d95/Computer_Science/My_Hobby_Projects/Live_Streaming/P2P_Live_Streaming/3rdparty/StunUsernameAttribute.h
  - C++, each clean example, a couple of generic attribute values: https://github.com/sourcey/libsourcey/blob/f126fdaa26fdcc3f44ebd2d70f2915ee0b6a1b23/src/stun/tests/stuntests.cpp
  - Stun attribute values: http://www.iana.org/assignments/stun-parameters/stun-parameters.xhtml
  - Some good info on the attributes and their meaning: http://www.3cx.com/blog/voip-howto/stun-details/
  - Test data for message-integrity checks: http://tools.ietf.org/html/rfc5769
*/

#ifndef ICE_STUN_H
#define ICE_STUN_H

#include <stdint.h>
#include <vector>
#include <string>

namespace stun {

  class Attribute;

  enum MessageType {
    STUN_MSG_TYPE_NONE           = 0x0000,
    STUN_BINDING_REQUEST         = 0x0001,
    STUN_BINDING_RESPONSE        = 0x0101,
    STUN_BINDING_ERROR_RESPONSE  = 0x0111,
    STUN_BINDING_INDICATION      = 0x0011
  };

  enum AttributeType {
    STUN_ATTR_TYPE_NONE            = 0x0000,
    STUN_ATTR_MAPPED_ADDR          = 0x0001,
    STUN_ATTR_CHANGE_REQ           = 0x0003,
    STUN_ATTR_USERNAME             = 0x0006,
    STUN_ATTR_MESSAGE_INTEGRITY    = 0x0008,                 /* See: http://tools.ietf.org/html/rfc5389#section-15.4 + http://tools.ietf.org/html/rfc4013, SHA1, 20 bytes*/
    STUN_ATTR_ERR_CODE             = 0x0009,
    STUN_ATTR_UNKNOWN_ATTRIBUTES   = 0x000a,
    STUN_ATTR_CHANNEL_NUMBER       = 0x000c,
    STUN_ATTR_LIFETIME             = 0x000d,
    STUN_ATTR_XOR_PEER_ADDR        = 0x0012,
    STUN_ATTR_DATA                 = 0x0013,
    STUN_ATTR_REALM                = 0x0014,
    STUN_ATTR_NONCE                = 0x0015,
    STUN_ATTR_XOR_RELAY_ADDRESS    = 0x0016,
    STUN_ATTR_REQ_ADDRESS_FAMILY   = 0x0017,
    STUN_ATTR_EVEN_PORT            = 0x0018,
    STUN_ATTR_REQUESTED_TRANSPORT  = 0x0019,
    STUN_ATTR_DONT_FRAGMENT        = 0x001a,
    STUN_ATTR_XOR_MAPPED_ADDRESS   = 0x0020,
    STUN_ATTR_RESERVATION_TOKEN    = 0x0022,
    STUN_ATTR_PRIORITY             = 0x0024,                /* See: http://tools.ietf.org/html/rfc5245#section-7.1.2.1 */
    STUN_ATTR_USE_CANDIDATE        = 0x0025,                /* See: http://tools.ietf.org/html/rfc5245#section-7.1.2.1 */
    STUN_ATTR_PADDING              = 0x0026,
    STUN_ATTR_RESPONSE_PORT        = 0x0027,
    STUN_ATTR_SOFTWARE             = 0x8022,
    STUN_ATTR_ALTERNATE_SERVER     = 0x8023,
    STUN_ATTR_FINGERPRINT          = 0x8028,                 /* See: http://tools.ietf.org/html/rfc5389#section-8 + http://tools.ietf.org/html/rfc5389#section-15.5 */
    STUN_ATTR_ICE_CONTROLLED       = 0x8029,                 /* See: http://tools.ietf.org/html/rfc5245#section-19.1 */
    STUN_ATTR_ICE_CONTROLLING      = 0x802a,                 /* See: http://tools.ietf.org/html/rfc5245#section-19.1 */
    STUN_ATTR_RESPONSE_ORIGIN      = 0x802b,
    STUN_ATTR_OTHER_ADDRESS        = 0x802c,
  };

  /* --------------------------------------------------------------------- */
  std::string attribute_type_to_string(uint32_t t);
  std::string message_type_to_string(uint32_t t);

  /* --------------------------------------------------------------------- */
  
  class Message {
  public:
    Message();
    ~Message();
    void addAttribute(Attribute* attr);                     /* add an attribute to the message who takes ownership (will delete all attributes in the d'tor. */
  public:
    uint16_t type;
    uint16_t length;
    uint32_t cookie;
    uint32_t transaction[3];
    std::vector<Attribute*> attributes;
  };

  /* --------------------------------------------------------------------- */
  
  class Attribute {
  public:
    Attribute(uint16_t type = STUN_ATTR_TYPE_NONE);
    
  public:
    uint16_t type;
    uint16_t length;
  };


  class StringValue {
  public:
    std::vector<uint8_t> buffer;
  };

  class Username : public Attribute {
  public:
    Username():Attribute(STUN_ATTR_USERNAME){ }
    StringValue value;
  };


  /* --------------------------------------------------------------------- */

  class STUN {
  public:
    STUN();
    void process(uint8_t* data, uint32_t nbytes);

  private:
    uint16_t readU16();                   /* read an uint16_t from the buffer, expecting the buffer to hold Big Endian data and moving the dx member. */
    uint32_t readU32();                   /* read an uint32_t from the buffer, expecting the buffer to hold Big Endian data and moving the dx member. */
    uint64_t readU64();                   /* read an uint64_t from the buffer, expecting the buffer to hold Big Endian data and moving the dx member. */
    void skip(uint32_t nbytes);           /* skip the next nbytes. */
    StringValue readString(uint16_t len); /* read a StringValue from the current buffer */
    uint32_t bytesLeft();                 /* returns the number of bytes that still need to be parsed, this is not the same as the size of the buffer! */
    uint8_t* ptr();                       /* returns a pointer to the current read index of the buffer. */
    
  public:
    //StateSTUN state;
    std::vector<uint8_t> buffer;
    size_t dx;
  };

} /* namespace stun */

#endif
