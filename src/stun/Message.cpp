#include <stun/Message.h>
#include <stun/Utils.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  Message::Message(uint16_t type)
    :type(type)
    ,length(0)
    ,cookie(0x2112a442)
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

  void Message::setTransactionID(uint32_t a, uint32_t b, uint32_t c) {
    transaction[0] = a;
    transaction[1] = b;
    transaction[2] = c;
  }

  bool Message::find(MessageIntegrity** result) {
    return find<MessageIntegrity>(STUN_ATTR_MESSAGE_INTEGRITY, result);
  }

  bool Message::computeMessageIntegrity(std::string key) {

    MessageIntegrity* integ = NULL;
    size_t i;
    uint16_t msg_size = 24;
    uint8_t* size_ptr = (uint8_t*)&msg_size;
    uint8_t curr_size[2];

    if (!key.size()) { 
      printf("Error: cannot compute message integrity in stun::Message because the key is empty.\n");
      return false;
    }

    if (!attributes.size() || !find(&integ)) {
      printf("Error: cannot compute the message integrity in stun::Message because the message doesn't contain a MessageIntegrity attribute.\n");
      return false;
    }

    /* compute the size that should be used for as Message-Length when computing the HMAC-SHA1 */
    for( i = 0; i < attributes.size(); ++i) {
      if (attributes[i]->type == STUN_ATTR_MESSAGE_INTEGRITY) {
        break;
      }
      msg_size += attributes[i]->nbytes;
    }

    /* copy and rewrite the size */
    curr_size[0] = buffer[2];
    curr_size[1] = buffer[3];
    buffer[2] = size_ptr[1];    
    buffer[3] = size_ptr[0];

    /* compute the sha */
    return compute_hmac_sha1(&buffer[0], msg_size - 4, key, integ->sha1);
  }

} /* namespace stun */
