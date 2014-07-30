#include <stun/Reader.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  static bool stun_validate_cookie(uint32_t cookie);

  /* --------------------------------------------------------------------- */

  Reader::Reader() 
    :dx(0)
    ,on_message(NULL)
    ,user(NULL)
  {
  }

  /* @todo - we can optimize this part by not copying but setting pointers to the 
             members of the Message. */
  void Reader::process(uint8_t* data, uint32_t nbytes) {

    if (!data) {
      printf("Error: received invalid data in Reader::process().\n");
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
    std::copy(data, data + nbytes, std::back_inserter(msg.buffer));

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

    if (on_message) {
      on_message(&msg, user);
    }
  }

  uint32_t Reader::bytesLeft() {
    if (buffer.size() == 0) { 
      return 0;
    }

    return buffer.size() - dx;
  }

  uint16_t Reader::readU16() {

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

  uint32_t Reader::readU32() {

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

  void Reader::skip(uint32_t nbytes) {
    if (dx + nbytes > buffer.size()) {
      printf("Error: trying to skip %u bytes, but we only have %u left in STUN.\n", nbytes, bytesLeft());
      return;
    }
    dx += nbytes;
  }


  StringValue Reader::readString(uint16_t len) {
    StringValue v;

    if (bytesLeft() < len) {
      printf("Error: trying to read a StringValue from the buffer, but the buffer is not big enough.\n");
      return v;
    }

    std::copy(ptr(), ptr() + len, std::back_inserter(v.buffer));
    dx += len;
    return v;
  }

  uint8_t* Reader::ptr() {
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

} /* namespace stun */
