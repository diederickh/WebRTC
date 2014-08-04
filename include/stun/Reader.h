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

#ifndef STUN_READER_H
#define STUN_READER_H


#include <stdint.h>
#include <vector>
#include <string>
#include <stun/Types.h>
#include <stun/Attribute.h>
#include <stun/Message.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  class Attribute;
  class Message;

  typedef void(*stun_on_message_callback)(Message* msg, void* user);                        /* is called when we successfully parse a stun messge. */
  typedef void(*stun_on_pass_through)(uint8_t* data, uint32_t nbytes, void* user);          /* is called when the data given to the reader is not a stun message */

  /* --------------------------------------------------------------------- */

  class Reader {

  public:
    Reader();
    void process(uint8_t* data, uint32_t nbytes);
    std::string password;

  private:
    uint8_t readU8();
    uint16_t readU16();                   /* read an uint16_t from the buffer, expecting the buffer to hold Big Endian data and moving the dx member. */
    uint32_t readU32();                   /* read an uint32_t from the buffer, expecting the buffer to hold Big Endian data and moving the dx member. */
    uint64_t readU64();                   /* read an uint64_t from the buffer, expecting the buffer to hold Big Endian data and moving the dx member. */
    StringValue readString(uint16_t len); /* read a StringValue from the current buffer */
    XorMappedAddress* readXorMappedAddress();
    void skip(uint32_t nbytes);           /* skip the next nbytes. */    
    uint32_t bytesLeft();                 /* returns the number of bytes that still need to be parsed, this is not the same as the size of the buffer! */
    uint8_t* ptr();                       /* returns a pointer to the current read index of the buffer. */
    
  public:
    stun_on_message_callback on_message;
    stun_on_pass_through on_pass_through;
    void* user;
    std::vector<uint8_t> buffer;
    size_t dx;
  };

} /* namespace stun */

#endif
