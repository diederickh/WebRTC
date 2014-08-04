#ifndef STUN_UTILS_H
#define STUN_UTILS_H

#include <stdint.h>
#include <string>
#include <vector>

namespace stun {
  
  /* 
     Compute the hmac-sha1 over message.

     uint8_t* message:  the data over which we compute the hmac sha
     uint32_t nbytes:   the number of bytse in message
     std::string key:   key to use for hmac 
     uint8_t* output:   we write the sha1 into this buffer.
   */
  bool compute_hmac_sha1(uint8_t* message, uint32_t nbytes, std::string key, uint8_t* output);

  /* 
     Compute the Message-Integrity of a stun message. 
     This will not change the given buffer.
     
     std::vector<uint8_t>& buffer: the buffer that contains a valid stun message 
     std::string key:              key to use for hmac 
     uint8_t* output:              will be filled with the correct hmac-sha1 of that represents the integrity message value. 
  */
  bool compute_message_integrity(std::vector<uint8_t>& buffer, std::string key, uint8_t* output);

  /* 
     Compute the fingerprint value for the stun message.
     This will not change the given buffer.

     std::vector<uint8_t>& buffer:    the buffer that contains a valid stun message.
     uint32_t& result:                will be set to the calculated crc value.
  */
  bool compute_fingerprint(std::vector<uint8_t>& buffer, uint32_t& result);


} /* namespace stun */

#endif
