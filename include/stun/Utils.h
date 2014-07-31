#ifndef STUN_UTILS_H
#define STUN_UTILS_H

#include <stdint.h>
#include <string>

namespace stun {
  
  /* 
     Computer the hmac-sha1 over message.

     uint8_t* message: the data over which we compute the hmac sha
     uint32_t nbytes: the number of bytse in message
     std::string key: key to use for hmac 
     uint8_t* output: we write the sha1 into this buffer.
   */
  bool compute_hmac_sha1(uint8_t* message, uint32_t nbytes, std::string key, uint8_t* output);

} /* namespace stun */

#endif
