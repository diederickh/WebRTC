#include <stun/Message.h>
#include <stun/Utils.h>
#include <zlib.h>

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

  bool Message::find(XorMappedAddress** result) {
    return find<XorMappedAddress>(STUN_ATTR_XOR_MAPPED_ADDRESS, result);
  }

  bool Message::find(Fingerprint** result) {
    return find<Fingerprint>(STUN_ATTR_FINGERPRINT, result);
  }

  bool Message::computeMessageIntegrity(std::string key) {

    MessageIntegrity* integ = NULL;
    if (!key.size()) { 
      printf("Error: cannot compute message integrity in stun::Message because the key is empty.\n");
      return false;
    }

    if (!attributes.size() || !find(&integ)) {
      printf("Error: cannot compute the message integrity in stun::Message because the message doesn't contain a MessageIntegrity attribute.\n");
      return false;
    }

    return compute_message_integrity(buffer, key, integ->sha1);
  }


  bool Message::computeFingerprint() {

    Fingerprint* finger = NULL;
    if (!attributes.size() || !find(&finger)) {
      printf("Error: cannot compute fingerprint because there is not fingerprint attribute.\n");
      return false;
    }

    return compute_fingerprint(buffer, finger->crc);
  }

} /* namespace stun */
