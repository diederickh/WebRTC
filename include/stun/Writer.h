/*

  stun::Writer
  -------------
  
  This will fill the `buffer` member with the binary representation
  of the values in the Message. It will also compute the message integrity
  and fingerprint CRC32 value when they're found in the attributes of the 
  Message that is passed into Writer::writeMessage().

 */
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
    void writeMessage(Message* msg, std::string messageIntegrityPassword);   /* When you call this, we assume that the message contains a MessageIntegrity attribute. We calculate the hmac-sha and rewrite our internal buffer. This also checks for a Fingerprint attribute; and computers + writes this crc32-value. */

  private:
    void writeAttribute(Attribute* attr);
    void writeUsername(Username* u);
    void writeSoftware(Software* s);
    void writePriority(Priority* p);
    void writeIceControlled(IceControlled* ic);
    void writeIceControlling(IceControlling* ic);
    void writeMessageIntegrity(MessageIntegrity* integ);
    void writeFingerprint(Fingerprint* fp);
    void writeXorMappedAddress(XorMappedAddress* xma);
    void writeU8(uint8_t v);
    void writeU16(uint16_t v);
    void writeU32(uint32_t v);
    void writeU64(uint64_t v);
    void writeBytes(uint8_t* buf, uint32_t nbytes);
    void writeString(StringValue v);
    void rewriteU16(size_t index, uint16_t v);
    void rewriteU32(size_t index, uint32_t v);
    
  public:
    std::vector<uint8_t> buffer;
  };

} /* namespace stun */
#endif
