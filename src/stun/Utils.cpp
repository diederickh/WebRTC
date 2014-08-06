#include <stdio.h>
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <zlib.h>  /* for crc */

#include <stun/Types.h>
#include <stun/Utils.h>

namespace stun {

  bool compute_hmac_sha1(uint8_t* message, uint32_t nbytes, std::string key, uint8_t* output) {

    if (!message) { 
      printf("Error: can't compute hmac_sha1 as the input message is empty in compute_hmac_sha1().\n");
      return false;
    }

    if (nbytes == 0) {
      printf("Error: can't compute hmac_sha1 as the input length is invalid in compute_hmac_sha1().\n");
      return false;
    }

    if (key.size() == 0) {
      printf("Error: can't compute the hmac_sha1 as the key size is 0 in compute_hmac_sha1().\n");
      return false;
    }

    if (!output) {
      printf("Error: can't compute the hmac_sha as the output buffer is NULL in compute_hmac_sha1().\n");
      return false;
    }

    unsigned int len;
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
  
    if (!HMAC_Init_ex(&ctx, (const unsigned char*)key.c_str(), key.size(), EVP_sha1(), NULL)) {
      printf("Error: cannot init the HMAC context in compute_hmac_sha1().\n");
    }

    HMAC_Update(&ctx, (const unsigned char*)message, nbytes);
    HMAC_Final(&ctx, output, &len);

#if 1
    printf("stun::compute_hmac_sha1 - verbose: computing hash over %u bytes, using key `%s`:\n", nbytes, key.c_str());
    printf("-----------------------------------\n\t0: ");
    int nl = 0, lines = 0;
    for (int i = 0; i < nbytes; ++i, ++nl) {
      if (nl == 4) {
        printf("\n\t");
        nl = 0;
        lines++;
        printf("%d: ", lines);
      }
      printf("%02X ", message[i]);
    }
    printf("\n-----------------------------------\n");
#endif

#if 1
    printf("stun::compute_hmac_sha1 - verbose: computed hash: ");
    for(unsigned int i = 0; i < len; ++i) {
      printf("%02X ", output[i]);
    }
    printf("\n");
#endif

    return true;
    
  }

  bool compute_message_integrity(std::vector<uint8_t>& buffer, std::string key, uint8_t* output) {

    uint16_t dx = 20;
    uint16_t offset = 0;
    uint16_t len = 0;
    uint16_t type = 0;
    uint8_t curr_size[2];

    if (!buffer.size()) {
      printf("Error: cannot compute message integrity; buffer empty.\n");
      return false;
    }

    if (!key.size()) {
      printf("Error: cannot compute message inegrity, key empty.\n");
      return false;
    }

    curr_size[0] = buffer[2];
    curr_size[1] = buffer[3];
    
    while (dx < buffer.size()) {

      type |= buffer[dx + 1] & 0x00FF;
      type |= (buffer[dx + 0] << 8) & 0xFF00;
      dx += 2;

      len |= (buffer[dx + 1] & 0x00FF);
      len |= (buffer[dx + 0] << 8) & 0xFF00;
      dx += 2;

      offset = dx;
      dx += len;

      /* skip padding. */
      while ( (dx & 0x03) != 0 && dx < buffer.size()) {
        dx++;
      }

      if (type == STUN_ATTR_MESSAGE_INTEGRITY) {
        break;
      }

      type = 0;
      len = 0;
    }

    /* rewrite Message-Length header field */
    buffer[2] = (offset >> 8) & 0xFF;
    buffer[3] = offset & 0xFF;

    /*
      and compute the sha1 
      we subtract the last 4 bytes, which are the attribute-type and
      attribute-length of the Message-Integrity field which are not
      used. 
    */
    if (!stun::compute_hmac_sha1(&buffer[0], offset - 4, key, output)) {
      buffer[2] = curr_size[0];
      buffer[3] = curr_size[1];
      return false;
    }

    /* rewrite message-length. */
    buffer[2] = curr_size[0];
    buffer[3] = curr_size[1];

    return true;
  }

  bool compute_fingerprint(std::vector<uint8_t>& buffer, uint32_t& result) {

    uint32_t dx = 20;
    uint16_t offset = 0;
    uint16_t len = 0;  /* messsage-length */
    uint16_t type = 0;
    uint8_t curr_size[2];

    if (!buffer.size()) {
      printf("Error: cannot compute fingerprint because the buffer is empty.\n");
    }

    /* copy current message-length */
    curr_size[0] = buffer[2];
    curr_size[1] = buffer[3];

    /* compute the size that should be used as Message-Length when computing the CRC32 */
    while (dx < buffer.size()) {
      
      type |= buffer[dx + 1] & 0x00FF;
      type |= (buffer[dx + 0] << 8) & 0xFF00;
      dx += 2;

      len |= buffer[dx + 1] & 0x00FF;
      len |= (buffer[dx + 0] << 8) & 0xFF00;
      dx += 2;

      offset = dx;
      dx += len;

      /* skip padding. */
      while ( (dx & 0x03) != 0 && dx < buffer.size()) {
        dx++;
      }

      if (type == STUN_ATTR_FINGERPRINT) {
        break;
      }

      type = 0;
      len = 0;
    }

    /* rewrite message-length */
    offset -= 16;
    buffer[2] = (offset >> 8) & 0xFF;
    buffer[3] = offset & 0xFF;

    result = crc32(0L, &buffer[0], offset + 12) ^ 0x5354554e;
    
    /* and reset the size */
    buffer[2] = curr_size[0];
    buffer[3] = curr_size[1];


    return true;
  }

} /* namespace stun */
