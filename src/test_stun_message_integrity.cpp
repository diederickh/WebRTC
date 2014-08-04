/*
  
  Calculating the HMAC-SHA1 message integrity attribute:
  ------------------------------------------------------

  1. Loop over all attributes and keep track of how many bytes
  you've read from the buffer. When you arrive at the MESSAGE-INTEGRITY
  attribute you need to remember how many bytes you've read so far.
     
  2. Next you need to rewrite the Message-Length field of the
  stun buffer because the HMAC-SHA1 needs to be computed with a 
  STUN buffer that contains the stun header with as message length
  that is computed up to, and including the size of the MESSAGE-INTEGRITY
  attribute. Sometimes a stun message can contain a FINGERPRINT attribute
  after the MESSAGE-INTEGRITY attribute and the Message-Length field 
  will also include the size of this attribute (in practice there 
  may be even more other attributes).

  e.g.

  [ STUN HEADER ]                     - 20 bytes
  [ ATTRIBUTE 1 ]                     - 8 bytes
  [ ATTRIBUTE 2 ]                     - 8 bytes 
  [ MESSAGE-INTEGRITY-ATTRIBUTE ]     - 24 bytes (2 bytes type, 2 bytes size, 20 bytes sha1)
  [ FINGERPRINT ]                     - 8 bytes

  In the message above, the [ STUN HEADER ] will contain a size value of:
  8 + 8 + 24 + 8, though to compute the SHA1, this should be 8 + 8 + 24 (not
  the FINGERPRINT value. 

  3. After rewriting the size, you use all bytes up to, BUT NOT including
  the MESSAGE-INTEGRITY attribute as data to compute the SHA1 

  It's important to know that you need to use the ice-pwd from the 
  other party as key to calculate the hmac-sha1. 

  N.B. be aware of the 32-bit memory alignment


*/
#include <stdio.h>
#include <vector>
#include <stun/Utils.h>
#include <stun/Reader.h>

#define USE_WEBRTC 1
#if USE_WEBRTC
#  define PASSWD "Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC"

#else 
#  define PASSWD "VOkJxbRl1RmTxUk/WvJxBt"
#endif

static void on_stun_message(stun::Message* msg, void* user);

int main() {

  printf("\n\ntest_stun_message_integrity\n\n");

  /* from: http://tools.ietf.org/html/rfc5769#section-2.2 */
#if USE_WEBRTC
  const unsigned char respv4[] = "\x00\x01\x00\x58\x21\x12\xa4\x42\x48\x75\x38\x77\x4e\x4f\x49\x53\x62\x54\x65\x4a\x00\x06\x00\x1b\x35\x50\x4e\x32\x71\x6d\x57\x71\x42\x6c\x3a\x34\x68\x52\x42\x6d\x66\x4b\x48\x75\x58\x6a\x4f\x67\x6b\x56\x4a\x00\x80\x2a\x00\x08\xd2\xdb\x70\x25\x70\x6b\xcd\x98\x00\x25\x00\x00\x00\x24\x00\x04\x6e\x7f\x1e\xff\x00\x08\x00\x14\x5a\xca\x22\xd7\xf4\x39\xfa\xde\xaf\xd3\x9b\xd6\xec\x00\xd4\x96\xe2\x17\x09\x32\x80\x28\x00\x04\x78\x5d\x2e\xeb";
#else
  const unsigned char respv4[] =
    "\x01\x01\x00\x3c"
    "\x21\x12\xa4\x42"
    "\xb7\xe7\xa7\x01\xbc\x34\xd6\x86\xfa\x87\xdf\xae"
    "\x80\x22\x00\x0b"
    "\x74\x65\x73\x74\x20\x76\x65\x63\x74\x6f\x72\x20"
    "\x00\x20\x00\x08"
    "\x00\x01\xa1\x47\xe1\x12\xa6\x43"
    "\x00\x08\x00\x14"
    "\x2b\x91\xf5\x99\xfd\x9e\x90\xc3\x8c\x74\x89\xf9"
    "\x2a\xf9\xba\x53\xf0\x6b\xe7\xd7"
    "\x80\x28\x00\x04"
    "\xc0\x7d\x4c\x96";
#endif

  int nl = 0;
  printf("\n--------\n");
  for (int i = 0; i < sizeof(respv4) - 1; ++i, ++nl) {

    if (nl == 4) {
      printf("\n");
      nl=0;
    }
    printf("%02X ", respv4[i]);
  }
  printf("\n--------\n");

  /* we use our reader to parse the message */
  stun::Reader reader;
  reader.on_message = on_stun_message;
  reader.process((uint8_t*)respv4, sizeof(respv4) - 1); /* we do -1 to exclude the string terminating nul char. */

  return 0;
}

static void on_stun_message(stun::Message* msg, void* user) {

  printf("Successfully parsed a stun message.\n");
  
  stun::MessageIntegrity* integ;
  if (msg->find(&integ)) { 
    
    /* We are rewriting the STUN Message-Length header here because it will contain the size 
       which includes the FINGERPRINT element that is stored in the test data. We loop over
       all attributes until we find the MESSAGE-INTEGRITY attribute. We start with a 
       size of 24 which is the size of the MESSAGE-INTEGRITY attribute which is also used
       in the Message-Length when we compute the HMAC-SHA1
    */

    uint16_t msg_size = 24;
    for (size_t i = 0; i < msg->attributes.size(); ++i) {
      if (msg->attributes[i]->type == stun::STUN_ATTR_MESSAGE_INTEGRITY) {
        break;
      }
      msg_size += msg->attributes[i]->nbytes;  /* nbytes contains the real number of bytes in a message, including the padding */
    }

    printf("%u\n", msg_size);
    
    /* Now we need to rewrite the Message-Length field big endian */
    uint8_t* new_len = (uint8_t*)& msg_size;
    msg->buffer[2] = new_len[1];
    msg->buffer[3] = new_len[0];

    uint8_t sha[20];
    stun::Reader reader;

    /* Ok, this part is a bit confusing. We have 2 different size variables to take into account:

       1. the size that is encoded in the Message-Lenth field which includes the number of bytes
       of all attributes up to and including the MESSAGE-INTEGRITY attribute which is 24 bytes.

       2. but we only need to compute the SHA using the bytes up to, but EXCLUDING the 
       MESSAGE-INTEGRITY element, but INCLUDING the Stun message header which is 20 bytes. 
       So here we simply subtract 4 bytes from the computed msg_size.
    */
    uint32_t data_size = msg_size - 4;
    stun::compute_hmac_sha1(&msg->buffer[0], data_size, PASSWD, sha);

    /* or we can use the build in function :-) */
    msg->computeMessageIntegrity(PASSWD);
  }
}
