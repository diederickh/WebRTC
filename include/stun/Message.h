#ifndef STUN_MESSAGE_H
#define STUN_MESSAGE_H

#include <stun/Attribute.h>
#include <stun/Types.h>

namespace stun {

  class Message {
  public:
    Message(uint16_t type = STUN_MSG_TYPE_NONE);
    ~Message();
    void addAttribute(Attribute* attr);                     /* add an attribute to the message who takes ownership (will delete all attributes in the d'tor. */
    void copyTransactionID(Message* from);                  /* copy the transaction ID from the given messsage */

  public:
    uint16_t type;
    uint16_t length;
    uint32_t cookie;
    uint32_t transaction[3];
    std::vector<Attribute*> attributes;
    std::vector<uint8_t> buffer;
  };


} /* namespace stun */

#endif
