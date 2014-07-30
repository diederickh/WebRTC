#include <ice/STUN.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  static bool stun_validate_cookie(uint32_t cookie);

  /* --------------------------------------------------------------------- */

  Attribute::Attribute(uint16_t type)
    :type(type)
    ,length(0)
  {
  }

  /* --------------------------------------------------------------------- */

  Message::Message()
    :type(STUN_MSG_TYPE_NONE)
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

  /* --------------------------------------------------------------------- */

  STUN::STUN() 
    :dx(0)
  {
  }

  void STUN::process(uint8_t* data, uint32_t nbytes) {

    if (!data) {
      printf("Error: received invalid data in STUN::process().\n");
      return;
    }

    if (nbytes < 2) {
      return;
    }

    /* @todo check first 2 bits */
    //    if (buffer[0] != 0x00 || buffer[1] != 0x00) {
    //      printf("Warning: no STUN message.\n");
    //      return;
    //    }

    std::copy(data, data + nbytes, std::back_inserter(buffer));
    
    /* A stun message must at least contain 20 bytes. */
    if (buffer.size() < 20) {
      return;
    }
    
    /* create the base message */
    Message msg;
    msg.type = readU16();
    msg.length = readU16();
    msg.cookie = readU32();
    msg.transaction[0] = readU32();
    msg.transaction[1] = readU32();
    msg.transaction[2] = readU32();

    /* validate */
    if (!stun_validate_cookie(msg.cookie)) {
      printf("Error: invalid STUN cookie.\n");
      return;
    }

    printf("Message length: %d\n", msg.length);

    /* parse the rest of the message */
    int c = 0;
    uint16_t attr_type;
    uint16_t attr_length;
    
    while (bytesLeft() >= 4) {

      Attribute* attr = NULL;
      attr_type = readU16();
      attr_length = readU16();

      printf("Msg: %s, Type: %s, Length: %d, bytes left: %u, current index: %ld\n", 
             message_type_to_string(msg.type).c_str(),
             attribute_type_to_string(attr_type).c_str(), 
             attr_length, 
             bytesLeft(), 
             dx);

#if 1      
      
      switch (attr_type) {

        /* no parsing needed for these */
        case STUN_ATTR_USE_CANDIDATE: {
          break;
        }

        case STUN_ATTR_USERNAME: { 
          Username* username = new Username();
          username->value = readString(attr_length);
          attr = (Attribute*) username;
          break;
        }


        case STUN_ATTR_PRIORITY: {
          skip(4); /* skip priority for now: http://tools.ietf.org/html/rfc5245#section-4.1.2.1 */
          break;
        } 

        case STUN_ATTR_MESSAGE_INTEGRITY: {     
          skip(20); /* SHA-1 message integrity */
          break;
        }

        case STUN_ATTR_FINGERPRINT: {
          skip(4); /* CRC32-bit, see http://tools.ietf.org/html/rfc5389#section-15.5 */
          break;
        }

        case STUN_ATTR_ICE_CONTROLLING:
        case STUN_ATTR_ICE_CONTROLLED: {
          skip(8);
          break;
        }

        default: {
          printf("Warning: unhandled STUN attribute %d of length: %u\n", attr_type, attr_length);
          break;
        }
      }

      if (attr) {
        msg.addAttribute(attr);
      }
#else 
      dx += attr_length;
#endif
      
      /* Padding: http://tools.ietf.org/html/rfc5389#section-15, must be 32bit aligned */
      while ( (dx & 0x03) != 0 && bytesLeft() > 0) {
        dx++;
      }
    }

    /* and erase any read data. */
    buffer.erase(buffer.begin(), buffer.begin() + dx);
    dx = 0;
  }

  uint32_t STUN::bytesLeft() {
    if (buffer.size() == 0) { 
      return 0;
    }

    return buffer.size() - dx;
  }

  uint16_t STUN::readU16() {

    if (bytesLeft() < 2) { 
      printf("Error: trying to readU16() in STUN, but the buffer is not big enough.\n");
      return 0;
    }

    uint16_t result;
    uint8_t* dst = (uint8_t*)&result;

    dst[0] = buffer[dx + 1];
    dst[1] = buffer[dx + 0];

    dx += 2;

    return result;
  }

  uint32_t STUN::readU32() {

    if (bytesLeft() < 4) {
      printf("Error: trying to readU32() in STUN, but the buffer is not big enough.\n");
      return 0;
    }

    uint32_t result;
    uint8_t* dst = (uint8_t*)&result;

    dst[0] = buffer[dx + 3];
    dst[1] = buffer[dx + 2];
    dst[2] = buffer[dx + 1];
    dst[3] = buffer[dx + 0];

    dx += 4;

    return result;
  }

  void STUN::skip(uint32_t nbytes) {
    if (dx + nbytes > buffer.size()) {
      printf("Error: trying to skip %u bytes, but we only have %u left in STUN.\n", nbytes, bytesLeft());
      return;
    }
    dx += nbytes;
  }


  StringValue STUN::readString(uint16_t len) {
    StringValue v;

    if (bytesLeft() < len) {
      printf("Error: trying to read a StringValue from the buffer, but the buffer is not big enough.\n");
      return v;
    }

    std::copy(ptr(), ptr() + len, std::back_inserter(v.buffer));
    dx += len;
    return v;
  }

  uint8_t* STUN::ptr() {
    return &buffer[dx];
  }

  /* --------------------------------------------------------------------- */

  static bool stun_validate_cookie(uint32_t cookie) {
    //printf("%02X:%02X:%02X:%02X\n", ptr[3], ptr[2], ptr[1], ptr[0]);
    uint8_t* ptr = (uint8_t*) &cookie;
    return (ptr[3] == 0x21 
            && ptr[2] == 0x12 
            && ptr[1] == 0xA4 
            && ptr[0] == 0x42);

    return true;
  }

  /* --------------------------------------------------------------------- */

  std::string attribute_type_to_string(uint32_t t) {
    switch (t) { 
      case STUN_ATTR_TYPE_NONE: { return "STUN_ATTR_TYPE_NONE"; }
      case STUN_ATTR_MAPPED_ADDR: { return "STUN_ATTR_MAPPED_ADDR"; }
      case STUN_ATTR_CHANGE_REQ: { return "STUN_ATTR_CHANGE_REQ"; }
      case STUN_ATTR_USERNAME: { return "STUN_ATTR_USERNAME"; }
      case STUN_ATTR_MESSAGE_INTEGRITY: { return "STUN_ATTR_MESSAGE_INTEGRITY"; }
      case STUN_ATTR_ERR_CODE: { return "STUN_ATTR_ERR_CODE"; }
      case STUN_ATTR_UNKNOWN_ATTRIBUTES: { return "STUN_ATTR_UNKNOWN_ATTRIBUTES"; }
      case STUN_ATTR_CHANNEL_NUMBER: { return "STUN_ATTR_CHANNEL_NUMBER"; }
      case STUN_ATTR_LIFETIME: { return "STUN_ATTR_LIFETIME"; }
      case STUN_ATTR_XOR_PEER_ADDR: { return "STUN_ATTR_XOR_PEER_ADDR"; }
      case STUN_ATTR_DATA: { return "STUN_ATTR_DATA"; }
      case STUN_ATTR_REALM: { return "STUN_ATTR_REALM"; }
      case STUN_ATTR_NONCE: { return "STUN_ATTR_NONCE"; }
      case STUN_ATTR_XOR_RELAY_ADDRESS: { return "STUN_ATTR_XOR_RELAY_ADDRESS"; }
      case STUN_ATTR_REQ_ADDRESS_FAMILY: { return "STUN_ATTR_REQ_ADDRESS_FAMILY"; }
      case STUN_ATTR_EVEN_PORT: { return "STUN_ATTR_EVEN_PORT"; }
      case STUN_ATTR_REQUESTED_TRANSPORT: { return "STUN_ATTR_REQUESTED_TRANSPORT"; }
      case STUN_ATTR_DONT_FRAGMENT: { return "STUN_ATTR_DONT_FRAGMENT"; }
      case STUN_ATTR_XOR_MAPPED_ADDRESS: { return "STUN_ATTR_XOR_MAPPED_ADDRESS"; }
      case STUN_ATTR_RESERVATION_TOKEN: { return "STUN_ATTR_RESERVATION_TOKEN"; }
      case STUN_ATTR_PRIORITY: { return "STUN_ATTR_PRIORITY"; }
      case STUN_ATTR_USE_CANDIDATE: { return "STUN_ATTR_USE_CANDIDATE"; }
      case STUN_ATTR_PADDING: { return "STUN_ATTR_PADDING"; }
      case STUN_ATTR_RESPONSE_PORT: { return "STUN_ATTR_RESPONSE_PORT"; }
      case STUN_ATTR_SOFTWARE: { return "STUN_ATTR_SOFTWARE"; }
      case STUN_ATTR_ALTERNATE_SERVER: { return "STUN_ATTR_ALTERNATE_SERVER"; }
      case STUN_ATTR_FINGERPRINT: { return "STUN_ATTR_FINGERPRINT"; }
      case STUN_ATTR_ICE_CONTROLLED: { return "STUN_ATTR_ICE_CONTROLLED"; }
      case STUN_ATTR_ICE_CONTROLLING: { return "STUN_ATTR_ICE_CONTROLLING"; }
      case STUN_ATTR_RESPONSE_ORIGIN: { return "STUN_ATTR_RESPONSE_ORIGIN"; }
      case STUN_ATTR_OTHER_ADDRESS: { return "STUN_ATTR_OTHER_ADDRESS"; }
      default: { return "unknown"; } 
    }
  }

  std::string message_type_to_string(uint32_t t) {
    switch (t) {
      case STUN_BINDING_REQUEST: { return "STUN_BINDING_REQUEST"; }
      case STUN_BINDING_RESPONSE: { return "STUN_BINDING_RESPONSE"; }
      case STUN_BINDING_ERROR_RESPONSE: { return "STUN_BINDING_ERROR_RESPONSE"; }
      case STUN_BINDING_INDICATION: { return "STUN_BINDING_INDICATION"; }
      default: { return "unknown"; } 
    }
  }

} /* namespace stun */
