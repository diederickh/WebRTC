#include <sstream>
#include <dtls/Context.h>

static int dtls_context_ssl_verify_peer(int ok, X509_STORE_CTX* ctx) ;

namespace dtls {

  Context::Context() 
    :cert(NULL)
    ,pkey(NULL)
    ,ctx(NULL)
  {

    if (SSL_library_init() != 1) {
      printf("Error: cannot initialize the SSL library.\n");
    }

  }

  Context::~Context() {

    if (cert) {
      X509_free(cert);
      cert = NULL;
    }

    if (pkey) {
      EVP_PKEY_free(pkey);
      pkey = NULL;
    }

    if (ctx) {
      SSL_CTX_free(ctx);
      ctx = NULL;
    }
  }


  bool Context::init() {

    if (!createKeyAndCertificate()) {
      return false;
    }

    
    if (!createContext()) {
      return false;
    }

    return true;
  }

  bool Context::init(std::string certfile, std::string keyfile) {

    if (!loadCertificateFile(certfile)) {
      return false;
    }
    
    if (!loadPrivateKeyFile(keyfile)) {
      return false;
    }

    if (!createContext()) {
      return false;
    }

    return true;
  }

  SSL* Context::createSSL() {

    if (!ctx) {
      printf("Warning: cannot create SSL() because we didn't find a valid SSL_CTX.\n");
      return NULL;
    }

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
      printf("Error: SSL_new() return an invalid pointer.\n");
    }

    return ssl;
  }

  bool Context::loadCertificateFile(std::string certfile) {

    if (0 == certfile.size()) {
      printf("Error: certificate file empty in dtls::Context::loadCertificateFile().\n");
      return false;
    }

    FILE* fp = fopen(certfile.c_str(), "r");
    if (!fp) {
      printf("Error: cannot load the certificate file: %s\n", certfile.c_str());
      return false;
    }

    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    if (!cert) {
      printf("Error: cannot read X509 in dtls::Context::loadCertificateFile().\n");
      fclose(fp);
      return false;
    }

    fclose(fp);
    fp = NULL;
    return true;
  }

  bool Context::loadPrivateKeyFile(std::string keyfile) {

    if (0 == keyfile.size()) {
      printf("Error: key file empty in dtls::Context::loadPrivateKey().\n");
      return false;
    }

    FILE* fp = fopen(keyfile.c_str(), "r");
    if (!fp) {
      printf("Error: cannot load the certificate file: %s\n", keyfile.c_str());
      return false;
    }
    
    pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    if(!pkey) {
      printf("Error: cannot read the private key file: %s\n", keyfile.c_str());
      fclose(fp);
      fp = NULL;
      return false;
    }

    fclose(fp);
    fp = NULL;

    return true;
  }

  bool Context::createContext() {

    int r = 0;

    if (!cert) {
      printf("Error: cannot create SSL_CTX because no certificate has been set.\n");
      return false;
    }

    if (!pkey) {
      printf("Error: cannot create SSL_CTX because no private key has been set.\n");
      return false;
    }

    if (ctx) {
      printf("Error: SSL_CTX already created.\n");
      return false;
    }

    /* create SSL object with DTLS support. */
    ctx = SSL_CTX_new(DTLSv1_method());
    if (!ctx) {
      printf("Error: cannot create SSL_CTX.\n");
      return false;
    }

    /* set our supported ciphers */
    r = SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
    if(r != 1) {
      printf("Error: cannot set the cipher list.\n");
      ERR_print_errors_fp(stderr);
      return false;
    }

    /* enable srtp */
    r = SSL_CTX_set_tlsext_use_srtp(ctx, "SRTP_AES128_CM_SHA1_80");
    if(r != 0) {
      printf("Error: cannot setup srtp support in dtls::Context.\n");
      ERR_print_errors_fp(stderr);
      return false;
    }

    /* set certificate */
    r = SSL_CTX_use_certificate(ctx, cert);
    if (r != 1) {
      printf("Error: cannot set the certificate in dtls::Context.\n");
      ERR_print_errors_fp(stderr);
      return false;
    }

    /* set private key. */
    r = SSL_CTX_use_PrivateKey(ctx, pkey);
    if (r != 1) {
      printf("Error: cannot set the private key in dtls::Context.\n");
      ERR_print_errors_fp(stderr);
      return false;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, dtls_context_ssl_verify_peer);

    return true;
  }
    

  bool Context::createKeyAndCertificate() {

    if (cert) {
      printf("Error: certificate already set in DTLS.\n");
      return false;
    }

    if (pkey) {
      printf("Error: key already set in DTLS.\n");
      return false;
    }

    if (!createKey()) {
      return false;
    }

    if (!createCertificate()) {
      return false;
    }

    return true;
  }


  bool Context::createFingerprint(std::string& result) {
    uint8_t fingerprint[8192];
    char fingerprint_string[8192];
    int r, i;
    int pos = 0;
    uint32_t len = sizeof(fingerprint);
    uint32_t buflen = sizeof(fingerprint_string);
 
    /* Init out buffers to zero */
    memset(fingerprint, 0x00, sizeof(fingerprint));
    memset(fingerprint_string, 0x00, sizeof(fingerprint_string));
 
    /* Get the digest */
    r = X509_digest(cert, EVP_sha256(), fingerprint, &len);
    if (r != 1) {
      printf("Error: cannot get digest from certificate.\n");
      return false;
    }
 
    for(i = 0; i < len; ++i) {
      if (i > 0) {
        pos += snprintf(fingerprint_string + pos, buflen - pos, ":");
      }
      pos += snprintf(fingerprint_string + pos, buflen - pos, "%02X", fingerprint[i]);
    }
 
    std::copy(fingerprint_string, fingerprint_string + pos, std::back_inserter(result));

    return true;
  }

  bool Context::createKey() {

    if (pkey) {
      printf("Error: key already creatd in DTLS.\n");
      return false;
    }

    pkey = EVP_PKEY_new();
    if (!pkey) {
      printf("Error: cannot allocate a EVP_PKEY in DTLS.\n");
      return false;
    }
 
    /* Generate the RSA key and assign it to the pkey. The `rsa` will be freed when we free the pkey. */
    RSA* rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
    if (!EVP_PKEY_assign_RSA(pkey, rsa)) {
      printf("Error: cannot assign the RSA key to our pkey in DTLS.\n");
      EVP_PKEY_free(pkey);
      return false;
    }    

    return true;
  }

  bool Context::createCertificate() {

    if (cert) { 
      printf("Error: certificate already created in DTLS.\n");
      return false;
    }

    if (!pkey) {
      printf("Error: cannot create a certificate, first create the key (in DTLS).\n");
      return false;
    }

    cert = X509_new();
    if (!cert) {
      printf("Error: cannot create the X509 structure in DTLS.\n");
      return false;
    }
 
    /* Set the serial number. Some browsers don't accept the default one (0).*/
    ASN1_INTEGER_set(X509_get_serialNumber(cert), 1);
 
    /* The certificate is valid until one year from now. */
    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), 31536000L);
 
    /* Set the public key for our certificate */
    X509_set_pubkey(cert, pkey);
 
    /* We want to copy the subject name to the issuer name. */
    X509_NAME* name = X509_get_subject_name(cert);
    if (!name) {
      printf("Error: cannot get x509 subject name in DTLS.\n");
      return false;
    }
 
    /* Set the country code and common name. */
    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char*)"NL",        -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char*)"roxlu",     -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"localhost", -1, -1, 0);
 
    /* Set the issuer name. */
    X509_set_issuer_name(cert, name);
 
    /* Sign the certificate with our key. */
    if (!X509_sign(cert, pkey, EVP_sha1())) {
      printf("Error: cannot sign the certificate in DTLS.\n");
      X509_free(cert);
      return false;
    }

    return true;
  }

} /* namespace dtls */


static int dtls_context_ssl_verify_peer(int ok, X509_STORE_CTX* ctx) {
  return 1;
}

