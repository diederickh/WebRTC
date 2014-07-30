/*

  test_ssl_fingerprint
  --------------------

  This code shows the following and is a prep for the DTLS in WebRTC:

  - use openssl to generate a key
  - create a self signed certificate
  - extract the fingerprint from the key

  References
  ----------
  - Example code + indepth explanation by Nathan: http://stackoverflow.com/questions/256405/programmatically-create-x509-certificate-using-openssl  
  - Example code with a bit more error checking then Nathans example: http://www.codepool.biz/security/how-to-use-openssl-to-sign-certificate.html
  - Example code by David M. Syzdek on how to create the fingerprint (sha1): http://stackoverflow.com/questions/9749560/how-to-calculate-x-509-certificates-sha-1-fingerprint-in-c-c-objective-c
  - Example by myself I worked on way back based on another example: https://gist.github.com/roxlu/4cc4534be340c82433db

 */
#include <string.h>
#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

struct app {
  X509* cert;
  EVP_PKEY* pkey;
};

static app* app_create();
static int app_create_key(app* a);
static int app_create_x509(app* a);
static int output_fingerprint(X509* cert);
static void cleanup(EVP_PKEY* key, X509* cert);

int main() {
  printf("\n\ntest_ssl_fingerprint\n\n");

  /* create our context. */
  app* a = app_create();
  if (!a) {
    exit(1);
  }

  if (app_create_key(a) < 0) {
    exit(1);
  }

  if (app_create_x509(a) < 0) {
    exit(1);
  }

  output_fingerprint(a->cert);

  cleanup(a->pkey, a->cert);

  return 0;
}


static app* app_create() {

  app* a = (app*) malloc(sizeof(app));
  if (!a) { 
    printf("Error: cannot create application.\n");
    return NULL;
  }

  a->cert = NULL;
  a->pkey = NULL;

  if (SSL_library_init() != 1) {
    printf("Error: cannot initialize the SSL library.\n");
    free(a);
    return NULL;
  }

  return a;
}

static int app_create_key(app* a) {
  if (!a) { return -1; } 

  a->pkey = EVP_PKEY_new();
  if (!a->pkey) {
    printf("Error: cannot allocate a EVP_PKEY.\n");
    return -2;
  }

  /* Generate the RSA key and assign it to the pkey. The `rsa` will be freed when we free the pkey. */
  RSA* rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
  if (!EVP_PKEY_assign_RSA(a->pkey, rsa)) {
    printf("Error: cannot assign the RSA key to our pkey.\n");
    EVP_PKEY_free(a->pkey);
    return -3;
  }

  return 0;
}

static int app_create_x509(app* a) {

  if (!a) { return -1; }
  if (!a->pkey) { return -2; } 
  
  a->cert = X509_new();
  if (!a->cert) {
    printf("Error: cannot create the X509 structure.\n");
    return -3;
  }

  /* Set the serial number. Some browsers don't accept the default one (0).*/
  ASN1_INTEGER_set(X509_get_serialNumber(a->cert), 1);

  /* The certificate is valid until one year from now. */
  X509_gmtime_adj(X509_get_notBefore(a->cert), 0);
  X509_gmtime_adj(X509_get_notAfter(a->cert), 31536000L);

  /* Set the public key for our certificate */
  X509_set_pubkey(a->cert, a->pkey);

  /* We want to copy the subject name to the issuer name. */
  X509_NAME* name = X509_get_subject_name(a->cert);
  if (!name) {
    printf("Error: cannot get x509 subject name.\n");
    return -4;
  }

  /* Set the country code and common name. */
  X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char*)"NL",        -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char*)"roxlu",     -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"localhost", -1, -1, 0);

  /* Set the issuer name. */
  X509_set_issuer_name(a->cert, name);

  /* Sign the certificate with our key. */
  if (!X509_sign(a->cert, a->pkey, EVP_sha1())) {
    printf("Error: cannot sign the certificate.\n");
    X509_free(a->cert);
    return -5;
  }

  return 0;
}

static int output_fingerprint(X509* cert) {
  if (!cert) { return -1; } 

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
    return -2;
  }

  printf("len: %d\n", len);

  for(i = 0; i < len; ++i) {
    if (i > 0) {
      pos += snprintf(fingerprint_string + pos, buflen - pos, ":");
    }
    pos += snprintf(fingerprint_string + pos, buflen - pos, "%02X", fingerprint[i]);
  }

  printf("a=fingerprint:sha-256 %s\n", fingerprint_string);

  return 0;
}

static void cleanup(EVP_PKEY* key, X509* cert) {
  if (cert) {
    X509_free(cert);
  }
  if (key) {
    EVP_PKEY_free(key);
  }
}
