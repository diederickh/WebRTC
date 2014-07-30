#ifndef RTC_DTLS_H
#define RTC_DTLS_H

#include <stdint.h>
#include <stdio.h>
#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <string>

/* SSL debug */
#define SSL_WHERE_INFO(ssl, w, flag, msg) {              \
    if(w & flag) {                                       \
      printf("----- ");                                  \
      printf("%20.20s", msg);                            \
      printf(" - %30.30s ", SSL_state_string_long(ssl)); \
      printf(" - %5.10s ", SSL_state_string(ssl));       \
      printf("\n");                                      \
    }                                                    \
  } 
 
namespace rtc {

  enum StateDTLS {
    DTLS_STATE_NONE,
    DTLS_STATE_HANDSHAKE_DONE
  };

  class DTLS {
  public:
    DTLS();
    ~DTLS();
    bool createKeyAndCertificate();                     /* creates a self signed certificate and private key */
    bool createFingerprint(std::string& result);        /* returns the fingerprint for the certificate */

  private:
    bool createKey();                                   /* creates a EVP_PKEY that is used to store private keys */
    bool createCertificate();                           /* creates the X509 certificate using EVP_PKEY member */

  public:
    StateDTLS state;
    X509* cert;
    EVP_PKEY* pkey;
  };

} /* namespace rtc */

#endif
