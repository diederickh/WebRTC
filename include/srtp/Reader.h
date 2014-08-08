/*


  References: 
  ----------

  - https://gist.github.com/roxlu/aa6d12653e10a91c9ddc
  - https://github.com/traviscross/baresip/blob/8974d662c942b10a9bb05223ddc7881896dd4c2f/modules/dtls_srtp/tls_udp.c
 */
#ifndef SRTP_READER_H
#define SRTP_READER_H

#include <stdint.h>
#include <srtp/srtp.h>

#define SRTP_READER_MASTER_KEY_LEN  16
#define SRTP_READER_MASTER_SALT_LEN 14
#define SRTP_READER_MASTER_LEN (SRTP_READER_MASTER_KEY_LEN + SRTP_READER_MASTER_SALT_LEN)

namespace srtp {

  class Reader {
  public:
    Reader();
    ~Reader();
    bool init(uint8_t* key, uint8_t* salt); 
    int process(uint8_t* data, uint32_t nbytes);          /* process raw incoming bytes with SRTP data. returns the number of bytes in the rtp packet or < 0 on error. */

  public:
    srtp_t session;
    srtp_policy_t policy;
    bool is_initialized;
  };

} /* namespace srtp */

#endif
