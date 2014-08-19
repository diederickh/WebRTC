/*

  test_extract_keying_info_for_srtp
  ---------------------------------

  This test shows how one can use openSSL to extract the keying material
  that can be used with SRTP. WebRTC uses DTLS to exchange the keying material. 
  In this file we simulate a DTLS client and server which perform the handshake and 
  exchange cryptographic parameters; once that's done we extract the keying
  material and set the correct key/salts for the client and server. 

  See http://tools.ietf.org/html/rfc5764#section-4.2 for a description 
  on how to use the keying material.

  ----------------------------------
  Related SSL functions:
    - SSL_get_selected_srtp_profile() - returns e.g. SRTP_AES128_CM_SHA1_80
    - SSL_CIPHER_get_name(SSL_get_current_cipher(ssl)) - returns e.g. AES256-SHA 
  ----------------------------------

  Create server/client self-signed certificate/key (self signed, DON'T ADD PASSWORD) 

  --
        openssl req -x509 -newkey rsa:2048 -days 3650 -nodes -keyout client-key.pem -out client-cert.pem
        openssl req -x509 -newkey rsa:2048 -days 3650 -nodes -keyout server-key.pem -out server-cert.pem
  -- 

  N.B: I'm no security or openSSL expoet so use this code at own risk.

 */

#include <stdio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

/* SRTP keying material sizes. */
#define SRTP_MASTER_KEY_LEN 16
#define SRTP_MASTER_SALT_LEN 14
#define SRTP_MASTER_LEN (SRTP_MASTER_KEY_LEN + SRTP_MASTER_SALT_LEN)

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

struct Agent {
  SSL_CTX* ctx;
  SSL* ssl;
  int is_server;
  BIO* in_bio;
  BIO* out_bio;
  unsigned char* remote_key;
  unsigned char* remote_salt;
  unsigned char* local_key;
  unsigned char* local_salt;
};

struct App {
  EVP_PKEY* pkey;
  X509* cert;
};

static int initialize_app(App* app);
static int initialize_agent(App* app, Agent* agent, int isserver);
static int check_output_buffer(Agent* readagent, Agent* writeagent);
static int check_input_buffer(Agent* readagent, Agent* writeagent);
static int dtls_context_ssl_verify_peer(int ok, X509_STORE_CTX* ctx);
static void dtls_parse_ssl_info_callback(const SSL* ssl, int where, int ret);

App application;
Agent client;
Agent server;

int main() {
  printf("\n\ntest_extract_keying_info_for_srtp.\n\n");

  /* INITIALIZE AGENTS + APPLICATION */
  /* ------------------------------- */

  if (initialize_app(&application) < 0) {
    exit(1);
  }

  if (initialize_agent(&application, &client, 0) < 0) {
    exit(1);
  }

  if (initialize_agent(&application, &server, 1) < 0) {
    exit(1);
  }

  /* PEFORM THE HANDSHAKE */
  /* -------------------- */

  while (!SSL_is_init_finished(client.ssl) || !SSL_is_init_finished(server.ssl)) {
    printf("Processing client:\n");
    printf("------------------------------------------------------\n");
    {
      SSL_do_handshake(client.ssl);
      check_output_buffer(&client, &server);
    }
    printf("\n");

    printf("Processing server:\n");
    printf("------------------------------------------------------\n");
    {
      SSL_do_handshake(server.ssl);
      check_output_buffer(&server, &client);
    }
    printf("\n");
  }

  /* EXTRACT KEYING INFORMATION */
  /* -------------------------- */

  /* will hold all of the keying data (2 pairs of key/salt, one for the client the other for server) */
  unsigned char keying_material[SRTP_MASTER_LEN * 2];  /* @todo ADD * 2 !! (testing debugger) */
  int r = 0;
  

  r = SSL_export_keying_material(client.ssl, 
                                 keying_material, 
                                 SRTP_MASTER_LEN * 2,
                                 "EXTRACTOR-dtls_srtp",
                                 19,
                                 NULL, 
                                 0,
                                 0);
  
  if (r != 1) {
    printf("Error: cannot export the keying material.\n");
    exit(1);
  }
  
  /* set the keying material for the client. */
  server.remote_key = keying_material;
  server.local_key = server.remote_key + SRTP_MASTER_KEY_LEN;
  server.remote_salt = server.local_key + SRTP_MASTER_KEY_LEN;
  server.local_salt = server.local_salt + SRTP_MASTER_SALT_LEN;

  /* set the keying material for the server. */
  client.local_key = keying_material;
  client.remote_key = client.local_key + SRTP_MASTER_KEY_LEN;
  client.local_salt = client.remote_key + SRTP_MASTER_KEY_LEN;
  client.remote_salt = client.local_salt + SRTP_MASTER_SALT_LEN;
  
  return 0;
}

static int initialize_app(App* app) {
  FILE* fp;

  if (!app) { return -1; } 

  SSL_library_init();
  
  
  /* LOAD PRIVATE KEY */
  {
    /* read private key file. (w/o password) */
    fp = fopen("./server-key.pem", "r");
    if (!fp) {
      printf("Error: can't read server-key.pem.\n");
      exit(1);
    }

    /* try to load */
    app->pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    if (!app->pkey) {
      printf("Error: cannot load private key.\n");
      exit(1);
    }

    fclose(fp);
    fp = NULL;
  }
  printf("Loaded private key.\n");

  /* LOAD CERTIFICATE */
  {
    /* read certificate */
    fp = fopen("./server-cert.pem", "r");
    if (!fp) {
      printf("Error: cannot load certificate.\n");
      exit(1);
    }
    
    app->cert = PEM_read_X509(fp, NULL, NULL, NULL);
    if (!app->cert) {
      printf("Error: cannot read certificate.\n");
      exit(1);
    }
    
    fclose(fp);
    fp = NULL;
  }

  printf("Loaded certificate.\n");

  return 0;
}

static int initialize_agent(App* app, Agent* agent, int isserver) {
  int r;

  if (!agent) { return -1; } 
  if (!app) { return -2; } 

  agent->is_server = isserver;
  
  /* INITIALIZE SSL CONTEXT */
  /* ---------------------- */

  if (agent->is_server) {
    agent->ctx = SSL_CTX_new(DTLSv1_server_method());
  }
  else {
    agent->ctx = SSL_CTX_new(DTLSv1_client_method());
  }

  r = SSL_CTX_set_cipher_list(agent->ctx, "ALL:NULL:eNULL:aNULL");
  if(r != 1) {
    printf("Error: cannot set the cipher list.\n");
    ERR_print_errors_fp(stderr);
    return -3;
  }

  SSL_CTX_set_options(agent->ctx, SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_TICKET | SSL_OP_SINGLE_ECDH_USE);
  SSL_CTX_set_session_cache_mode(agent->ctx, SSL_SESS_CACHE_OFF); 
  SSL_CTX_set_mode(agent->ctx, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_AUTO_RETRY);

  /* enable srtp */
  r = SSL_CTX_set_tlsext_use_srtp(agent->ctx, "SRTP_AES128_CM_SHA1_80");
  if(r != 0) {
    printf("Error: cannot setup srtp support.\n");
    ERR_print_errors_fp(stderr);
    return -4;
  }

  /* set certificate */
  r = SSL_CTX_use_certificate(agent->ctx, app->cert);
  if (r != 1) {
    printf("Error: cannot set the certificate.\n");
    ERR_print_errors_fp(stderr);
    return -5;
  }

  /* set private key. */
  r = SSL_CTX_use_PrivateKey(agent->ctx, app->pkey);
  if (r != 1) {
    printf("Error: cannot set the private key.\n");
    ERR_print_errors_fp(stderr);
    return -6;
  }

  SSL_CTX_set_verify(agent->ctx, SSL_VERIFY_PEER, dtls_context_ssl_verify_peer);

  agent->ssl = SSL_new(agent->ctx);
  if (!agent->ssl) {
    printf("Error: cannot create new SSL*.\n");
  }

  /* INITIALIZE BIOS */
  /* --------------- */

  /* in bio */
  {
    agent->in_bio = BIO_new(BIO_s_mem());
    if (!agent->in_bio) {
      printf("Error: agent failed because we can't create our in_bio.\n");
      return -7;
    }

    BIO_set_mem_eof_return(agent->in_bio, -1); /* see: https://www.openssl.org/docs/crypto/BIO_s_mem.html */
  }

  /* out bio */
  {
    agent->out_bio = BIO_new(BIO_s_mem());
    if (!agent->out_bio) {
      printf("Error: failed because can't create out out_bio.\n");
      /* @todo cleanup. */
      return -8;
    }

    BIO_set_mem_eof_return(agent->out_bio, -1); /* see: https://www.openssl.org/docs/crypto/BIO_s_mem.html */
  }

  /* set info callback */
  SSL_set_info_callback(agent->ssl, dtls_parse_ssl_info_callback);

  /* set in and output bios. */
  SSL_set_bio(agent->ssl, agent->in_bio, agent->out_bio);

  if (agent->is_server) {
    SSL_set_accept_state(agent->ssl); /* in case we're a server */
  }
  else {
    SSL_set_connect_state(agent->ssl); /* in case of a client */
  }

  return 0;
}

static int check_output_buffer(Agent* readagent, Agent* writeagent) {
  if (!readagent) { return -1; } 
  if (!writeagent) { return -2; } 
  
  char buffer[2048] = { 0 } ;
  int to_read = 0; 
  int nread = 0;

  /* is there data we need to send? */
  int pending = BIO_ctrl_pending(readagent->out_bio);
  printf("Pending in output buffer: %d\n", pending);
  if (pending <= 0) {
    return -2;
  }

  while(pending) {

    /* how many bytes to read. */
    if (pending >= 2048) {
      to_read = 2048;
    }
    else {
      to_read = pending;
    }

    nread = BIO_read(readagent->out_bio, buffer, to_read);
    if (nread != to_read) {
      printf("Error: failed readig the necessary amount of bytes from the out bio.\n");
      exit(1);
    }

    printf("Process: %d bytes.\n", nread);
    if (nread) {
      BIO_write(writeagent->in_bio, buffer, nread);
    }
      
    pending -= to_read;
  }

  return 0;
}

static int dtls_context_ssl_verify_peer(int ok, X509_STORE_CTX* ctx) {
  printf("DTLS_CONTEXT_SSL_VERIFY_PEER\n");
  return 1;
}

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

