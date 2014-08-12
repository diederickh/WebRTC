#ifndef SRTP_PARSER_H
#define SRTP_PARSER_H

#include <stdint.h>
#include <srtp/srtp.h>

#define SRTP_PARSER_MASTER_KEY_LEN  16
#define SRTP_PARSER_MASTER_SALT_LEN 14
#define SRTP_PARSER_MASTER_LEN (SRTP_PARSER_MASTER_KEY_LEN + SRTP_PARSER_MASTER_SALT_LEN)


namespace srtp {

  class ParserSRTP {

  public:
    ParserSRTP();
    ~ParserSRTP();
    int init(const char* cipher, bool inbound, const uint8_t* key, const uint8_t* salt);
    int protectRTP(void* in, uint32_t nbytes);
    int protectRTCP(void* in, uint32_t nbytes);
    int unprotectRTP(void* in, uint32_t nbytes);
    int unprotectRTCP(void* in, uint32_t nbytes);

  public:
    static bool is_lib_init;
    bool is_init;
    srtp_t session;
    srtp_policy_t policy;
  };

} /* namespace srtp */

#endif
