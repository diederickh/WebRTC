#include <sstream>
#include <rtc/DTLS.h>

namespace rtc {

  DTLS::DTLS() 
    :cert(NULL)
    ,pkey(NULL)
  {

    if (SSL_library_init() != 1) {
      printf("Error: cannot initialize the SSL library.\n");
    }

  }

  DTLS::~DTLS() {

    if (cert) {
      X509_free(cert);
      cert = NULL;
    }

    if (pkey) {
      EVP_PKEY_free(pkey);
      pkey = NULL;
    }
  }

  bool DTLS::createKeyAndCertificate() {

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


  bool DTLS::createFingerprint(std::string& result) {
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

  bool DTLS::createKey() {

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

  bool DTLS::createCertificate() {

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

} /* namespace rtc */
