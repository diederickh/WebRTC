#include <stun/Writer.h>

namespace stun {

  void Writer::writeMessage(Message* msg) {

    /* message header (20 bytes) */
    writeU16(msg->type); 
    writeU16(0);                   /* length */
    writeU32(0x2112A442);          /* cookie */
    writeU32(msg->transaction[0]); /* transaction */
    writeU32(msg->transaction[1]); /* transaction */
    writeU32(msg->transaction[2]); /* transaction */

    /* @todo write attributes */
    for (size_t i = 0; i < msg->attributes.size(); ++i) {
      writeAttribute(msg->attributes[i]);
    }


    /* Padding: http://tools.ietf.org/html/rfc5389#section-15, must be 32bit aligned */
    while ( (buffer.size() & 0x03) != 0) {
      buffer.push_back(0x00);
    }

    /* rewrite the message size header element; is the size w/o the header. */
    if (buffer.size() > UINT16_MAX) {
      printf("Warning: message size is too large.\n");
      return;
    }

    uint16_t message_size = buffer.size() - 20;
    rewriteU16(2, message_size);
  }

  void Writer::writeAttribute(Attribute* attr) {
    switch (attr->type) {
      case STUN_ATTR_USERNAME: { 
        writeUsername(static_cast<Username*>(attr)); 
        break; 
      } 
      default: {
        break;
      }
    }
  }

  void Writer::writeUsername(Username* u) {
    writeU16(STUN_ATTR_USERNAME);
    writeU16(u->value.buffer.size());
    writeString(u->value);
  }

  void Writer::writeString(StringValue v) {
    std::copy(v.buffer.begin(), v.buffer.end(), std::back_inserter(buffer));
  }

  void Writer::writeU16(uint16_t v) {
    uint8_t* p = (uint8_t*)&v;
    buffer.push_back(p[1]);
    buffer.push_back(p[0]);
  }

  void Writer::writeU32(uint32_t v) {
    uint8_t* p = (uint8_t*)&v;
    buffer.push_back(p[3]);
    buffer.push_back(p[2]);
    buffer.push_back(p[1]);
    buffer.push_back(p[0]);
  }

  void Writer::rewriteU16(size_t dx, uint16_t v) {

    if (dx >= buffer.size()) {
      printf("Warning: trying to rewriteU16, but our buffer is too small to contain a u16.\n");
      return;
    }

    uint8_t* p = (uint8_t*) &v;
    buffer[dx + 0] = p[1];
    buffer[dx + 1] = p[0];
  }
  
} /* namespace stun */
