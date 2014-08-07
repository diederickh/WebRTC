/*
  
  dtls::Parser 
  -------------
  Used to parse incoming DTLS data and keeps track of the current state 
  of the SSL* member. It will call the `on_data()` callback whenever you need
  to send some data back to the end point for which you're using this parser.

 */
#ifndef DTLS_PARSER_H
#define DTLS_PARSER_H

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdint.h>

#define DTLS_BUFFER_SIZE        (1024 * 96)          /* used to copy data from the mem bio */

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

/* SRTP keying material sizes. */
#define DTLS_SRTP_MASTER_KEY_LEN 16
#define DTLS_SRTP_MASTER_SALT_LEN 14
#define DTLS_SRTP_MASTER_LEN (DTLS_SRTP_MASTER_KEY_LEN + DTLS_SRTP_MASTER_SALT_LEN)

namespace dtls {

  typedef void (*dtls_parser_on_data_callback)(uint8_t* data, uint32_t nbytes, void* user);     /* gets called when the parse has data ready that needs to be send back to the other party. */

  enum ParserState {
    DTLS_STATE_NONE,
  };

  enum ParserMode {
    DTLS_MODE_NONE,
    DTLS_MODE_CLIENT,
    DTLS_MODE_SERVER
  };

  class Parser {
  public:
    Parser();
    ~Parser();
    bool init();
    void process(uint8_t* data, uint32_t nbytes);               /* process some encrypted data */
    bool isHandshakeFinished();
    bool extractKeyingMaterial();                               /* only when the SSL handshake has finsihed, this will extract the keying material that is used by srtp. */

  private:
    void checkOutputBuffer();                                   /* checks is there is data in our out_bio and that we need to send something to the other party. */

  public:
    SSL* ssl;                                                   /* the SSL object that tracks state. must be set by user, we take ownership and free it in the d'tor. */
    BIO* in_bio;                                                /* we use memory read bios. */     
    BIO* out_bio;                                               /* we use memory write bios. */
    ParserState state;/* @todo - check if we can't use the ssl member to tack state. */                                          /* used to state and makes sure the on_data callback is called at the right time. */
    ParserMode mode;                                            /* is this a client or server implementation */
    uint8_t* buffer;                                            /* is used to copy data out our out_bio/in_bio */
    dtls_parser_on_data_callback on_data;                       /* is called when there is data that needs to be send to the other party */ 
    void* user;                                                 /* gets passed into the callbacks */
    uint8_t keying_material[DTLS_SRTP_MASTER_LEN * 2];          /* contains the keying material. */ 
    uint8_t* remote_key;                                        /* remote key, used by srtp, points into keying_material */
    uint8_t* remote_salt;                                       /* remote salt, used by srtp, points into keying_material */
    uint8_t* local_key;                                         /* local key, used by srtp, points into keying_material */
    uint8_t* local_salt;                                        /* local salt, used by srtp, points into keying_material */ 
  };

} /* namespace dtls */

#endif
