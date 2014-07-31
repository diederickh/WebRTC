#include <stdio.h>
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>


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
    printf("Computed Hash: ");
    for(unsigned int i = 0; i < len; ++i) {
      printf("%02X ", output[i]);
    }
    printf("\n");
#endif

    return true;
    
  }

} /* namespace stun */
