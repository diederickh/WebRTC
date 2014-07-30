#include <stun/Message.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  Message::Message(uint16_t type)
    :type(type)
    ,length(0)
  {
  }

  Message::~Message() {
    std::vector<Attribute*>::iterator it = attributes.begin();
    while (it != attributes.end()) {
      delete *it;
      it = attributes.erase(it);
    }
  }

  void Message::addAttribute(Attribute* attr) {
    attributes.push_back(attr);
  }

  void Message::copyTransactionID(Message* from) {

    if (!from) {
      printf("Error: tryign to copy a transaction ID, but the message is invalid in stun::Message.\n");
      return;
    }

    transaction[0] = from->transaction[0];
    transaction[1] = from->transaction[1];
    transaction[2] = from->transaction[2];
  }

} /* namespace stun */
