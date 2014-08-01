#include <stun/Writer.h>
#include <arpa/inet.h> /* for inet_aton, not supported on windows, on win, use http://msdn.microsoft.com/en-us/library/cc805844%28VS.85%29.aspx */
#include <stdio.h>
#include <string>
#include <sstream>

namespace stun {

  void Writer::writeMessage(Message* msg) {

    /* first make sure our buffer is cleared. */
    buffer.clear();

    /* message header (20 bytes) */
    writeU16(msg->type); 
    writeU16(0);                   /* length */
    writeU32(0x2112A442);          /* cookie */
    writeU32(msg->transaction[0]); /* transaction */
    writeU32(msg->transaction[1]); /* transaction */
    writeU32(msg->transaction[2]); /* transaction */

    size_t prev_nbytes = buffer.size(); /* used to compute the total bytes an attribute takes up. */
    size_t prev_length = buffer.size(); /* used to compute the attribute length, excluding the attribute-type, attribute-length field and any padded bytes. */ 

    /* @todo write attributes */
    for (size_t i = 0; i < msg->attributes.size(); ++i) {

      prev_length = buffer.size();
      
      writeAttribute(msg->attributes[i]);

      msg->attributes[i]->length = (buffer.size() - prev_length) - 4; /* the -4 are the message-type and message-length bytes. */

      /* Padding: http://tools.ietf.org/html/rfc5389#section-15, must be 32bit aligned */
      while ( (buffer.size() & 0x03) != 0) {
        buffer.push_back(0x00);
      }

      msg->attributes[i]->nbytes = buffer.size() - prev_nbytes;

      prev_nbytes = buffer.size();
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

      case STUN_ATTR_SOFTWARE: {
        writeSoftware(static_cast<Software*>(attr));
        break;
      }

      case STUN_ATTR_PRIORITY: {
        writePriority(static_cast<Priority*>(attr));
        break;
      }

      case STUN_ATTR_ICE_CONTROLLED: {
        writeIceControlled(static_cast<IceControlled*>(attr));
        break;
      }

      case STUN_ATTR_ICE_CONTROLLING: {
        writeIceControlling(static_cast<IceControlling*>(attr));
        break;
      }

      case STUN_ATTR_MESSAGE_INTEGRITY: {
        writeMessageIntegrity(static_cast<MessageIntegrity*>(attr));
        break;
      }
        
      case STUN_ATTR_FINGERPRINT: {
        writeFingerprint(static_cast<Fingerprint*>(attr));
        break;
      }

      case STUN_ATTR_XOR_MAPPED_ADDRESS: {
        writeXorMappedAddress(static_cast<XorMappedAddress*>(attr));
        break;
      }

      default: {
        printf("Error: unhandled attribute in stun::Writer::writeAttribute(): %s\n", attribute_type_to_string(attr->type).c_str());
        break;
      }
    }
  }

  void Writer::writePriority(Priority* p) {
    writeU16(p->type);
    writeU16(4);       /* length */
    writeU32(p->value);
  }

  void Writer::writeUsername(Username* u) {
    writeU16(u->type);
    writeU16(u->value.buffer.size());
    writeString(u->value);
  }

  void Writer::writeSoftware(Software* s) {
    writeU16(s->type);
    writeU16(s->value.buffer.size());
    writeString(s->value);
  }

  void Writer::writeIceControlled(IceControlled* ic) {
    writeU16(ic->type);
    writeU16(8);
    writeU64(ic->tie_breaker);
  }

  void Writer::writeIceControlling(IceControlling* ic) {
    writeU16(ic->type);
    writeU16(8);
    writeU64(ic->tie_breaker);
  }

  void Writer::writeMessageIntegrity(MessageIntegrity* integ) {
    writeU16(integ->type);
    writeU16(20); /* size of sha1 */
    writeBytes(integ->sha1, 20);
  }

  void Writer::writeFingerprint(Fingerprint* fp) {
    writeU16(fp->type);
    writeU16(4);
    writeU32(fp->crc);
  }

  void Writer::writeXorMappedAddress(XorMappedAddress* xma) {

    /* write the header */
    writeU16(xma->type);
    writeU16(8);
    writeU8(0x00);
    writeU8(xma->family);

    /* calculate the xor mapped port and ip */
    uint32_t ip;
    uint32_t ip_copy;
    uint8_t* ip_ptr = (uint8_t*) &ip;
    uint8_t* ip_copy_ptr = (uint8_t*) &ip_copy;
    uint16_t port = xma->port;
    uint8_t* port_ptr = (uint8_t*)&port;
    uint8_t cookie[] = { 0x42, 0xA4, 0x12, 0x21 }; 
    
    /* xor the port */
    port_ptr[0] = port_ptr[0] ^ cookie[2];
    port_ptr[1] = port_ptr[1] ^ cookie[3];
 
    /* convert the address string into a uint32_t */
    inet_pton(AF_INET, xma->address.c_str(), ip_copy_ptr);
    
    /* xor the ip */
    ip_ptr[0] = ip_copy_ptr[3] ^ cookie[0];
    ip_ptr[1] = ip_copy_ptr[2] ^ cookie[1];
    ip_ptr[2] = ip_copy_ptr[1] ^ cookie[2];
    ip_ptr[3] = ip_copy_ptr[0] ^ cookie[3];

    writeU16(port);
    writeU32(ip);
  }

  void Writer::writeString(StringValue v) {
    std::copy(v.buffer.begin(), v.buffer.end(), std::back_inserter(buffer));
  }

  void Writer::writeBytes(uint8_t* bytes, uint32_t nbytes) {
    std::copy(bytes, bytes + nbytes, std::back_inserter(buffer));
  }

  void Writer::writeU8(uint8_t v) {
    buffer.push_back(v);
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

  void Writer::writeU64(uint64_t v) {
    uint8_t* p = (uint8_t*)&v;
    buffer.push_back(p[7]);
    buffer.push_back(p[6]);
    buffer.push_back(p[5]);
    buffer.push_back(p[4]);
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
