#include <dtls/Parser.h>

static void dtls_parse_ssl_info_callback(const SSL* ssl, int where, int ret);
//static void dtls_parse_ssl_verify_peer(int ok, X509_STORE_CTX* ctx);

namespace dtls {
  
  Parser::Parser()
    :ssl(NULL)
    ,state(DTLS_STATE_NONE)
    ,in_bio(NULL)
    ,out_bio(NULL)
    ,on_data(NULL)
    ,user(NULL)
  {
    buffer = new uint8_t[DTLS_BUFFER_SIZE];
    if (!buffer) {
      printf("Error: cannot allocate the out buffer in dtls::Parser. Out of mem?\n");
      exit(1);
    }
  }

  Parser::~Parser() {
    state = DTLS_STATE_NONE;

    if (ssl) {
      SSL_free(ssl);
      ssl = NULL;
    }

    if (buffer) {
      delete[] buffer;
      buffer = NULL;
    }

    /* @todo - free memory bios (in_bio and out_bio) */

    on_data = NULL;
    user = NULL;
  }

  bool Parser::init() {

    if (!ssl) {
      printf("Error: dtls::Parser::init() failed because the `ssl` member hasn't been set yet.\n");
      return false;
    }

    /* in bio */
    {
      in_bio = BIO_new(BIO_s_mem());
      if (!in_bio) {
        printf("Error: dtls::Parser::init() failed because we can't create our in_bio.\n");
        return false;
      }

      BIO_set_mem_eof_return(in_bio, -1); /* see: https://www.openssl.org/docs/crypto/BIO_s_mem.html */
    }

    /* out bio */
    {
      out_bio = BIO_new(BIO_s_mem());
      if (!out_bio) {
        printf("Error: dtls::Parser::init() failed because can't create out out_bio.\n");
        /* @todo cleanup. */
        return false;
      }

      BIO_set_mem_eof_return(out_bio, -1); /* see: https://www.openssl.org/docs/crypto/BIO_s_mem.html */
    }

    /* set info callback */
    SSL_set_info_callback(ssl, dtls_parse_ssl_info_callback);

    /* set in and output bios. */
    SSL_set_bio(ssl, in_bio, out_bio);

    SSL_set_accept_state(ssl); /* in case we're a server */
    //SSL_set_connect_state(ssl); /* in case we're a client */
    
    return true;
  }

  void Parser::process(uint8_t* data, uint32_t nbytes) {

    if (!in_bio) {
      printf("dtls::Parser - error: in_bio is invalid, not initialized?\n");
      return;
    }

    if (!data) {
      printf("Warning: calling Parser::process w/o valid data.\n");
      return;
    }

    if (!nbytes) {
      printf("Warning: calling Parser::process with invalid nbytes.\n");
      return;
    }

    int written = BIO_write(in_bio, data, nbytes);
    if (written > 0) {
      printf("Written: %d\n", written);
      if (!SSL_is_init_finished(ssl)) {
        SSL_do_handshake(ssl);
        checkOutputBuffer();
      }
    }
    else {
      printf("Error: %d\n", written);
    }

  }

  void Parser::checkOutputBuffer() {

    int to_read = 0; 
    int nread = 0;

    /* is there data we need to send? */
    int pending = BIO_ctrl_pending(out_bio);
    if (pending <= 0) {
      return;
    }

    while(pending) {

      /* how many bytes to read. */
      if (pending >= DTLS_BUFFER_SIZE) {
        to_read = DTLS_BUFFER_SIZE;
      }
      else {
        to_read = pending;
      }

      nread = BIO_read(out_bio, buffer, to_read);
      if (nread != to_read) {
        printf("Error: failed readig the necessary amount of bytes from the out bio.\n");
        exit(1);
      }
      
      if (on_data) {
        on_data(buffer, nread, user);
      }
      
      pending -= to_read;
    }
    
    printf("Pending in out_bio: %d\n", pending);
  }

} /* namespace dtls */


static void dtls_parse_ssl_info_callback(const SSL* ssl, int where, int ret) {

  if (ret == 0) {
    ERR_print_errors_fp(stderr);
    printf("dtls_parse_ssl_info_callback: error occured.\n");
    return;
  }

  SSL_WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
  SSL_WHERE_INFO(ssl, where, SSL_CB_EXIT, "EXIT");
  SSL_WHERE_INFO(ssl, where, SSL_CB_READ, "READ");
  SSL_WHERE_INFO(ssl, where, SSL_CB_WRITE, "WRITE");
  SSL_WHERE_INFO(ssl, where, SSL_CB_ALERT, "ALERT");
  SSL_WHERE_INFO(ssl, where, SSL_CB_READ_ALERT, "READ ALERT");
  SSL_WHERE_INFO(ssl, where, SSL_CB_WRITE_ALERT, "WRITE ALERT");
  SSL_WHERE_INFO(ssl, where, SSL_CB_ACCEPT_LOOP, "ACCEPT LOOP");
  SSL_WHERE_INFO(ssl, where, SSL_CB_ACCEPT_EXIT, "ACCEPT EXIT");
  SSL_WHERE_INFO(ssl, where, SSL_CB_CONNECT_LOOP, "CONNECT LOOP");
  SSL_WHERE_INFO(ssl, where, SSL_CB_CONNECT_EXIT, "CONNECT EXIT");
  SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_START, "HANDSHAKE START");
  SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
}

