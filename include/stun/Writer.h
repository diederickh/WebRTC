#ifndef STUN_WRITER_H
#define STUN_WRITER_H

#include <stdint.h>
#include <vector>
#include <string>
#include <stun/Attribute.h>
#include <stun/Message.h>
#include <stun/Types.h>

namespace stun {

  class Writer {
  public:
    void writeMessage(Message* msg);
    void writeAttribute(Attribute* attr);
    void writeUsername(Username* u);
    void writeU16(uint16_t v);
    void writeU32(uint32_t v);
    void writeString(StringValue v);
    void rewriteU16(size_t index, uint16_t v);
    
  public:
    std::vector<uint8_t> buffer;
    
  };

} /* namespace stun */
#endif
