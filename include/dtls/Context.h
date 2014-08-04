/*


  dtls::Context
  -------------
  
  Context for openSSL DTLS features. You can use this to craete a SSL_CTX and from 
  that instantiate SSL* objects that can be used to encrypt/decrypt your data. This 
  is created for the WebRTC DTLS part.  This class allows you to automatically generate
  a certificate and key or load them from file. 

  ** NOTE **
             @todo - update dtls::Context.h info when we added support for passwords.
  ** NOTE **

   Create server/client self-signed certificate/key (self signed, DONT ADD PASSWORD) 
   --
         openssl req -x509 -newkey rsa:2048 -days 3650 -nodes -keyout client-key.pem -out client-cert.pem
         openssl req -x509 -newkey rsa:2048 -days 3650 -nodes -keyout server-key.pem -out server-cert.pem
   -- 

 */
#ifndef DTLS_CONTEXT_H
#define DTLS_CONTEXT_H

#include <stdint.h>
#include <stdio.h>
#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <string>

namespace dtls {

  class Context {
  public:
    Context();
    ~Context();
    bool init();                                                             /* generates a certificate + private key on the fly */               
    bool init(std::string certfile, std::string keyfile);                    /* loads the given certificate + private key */
    bool createFingerprint(std::string& result);                             /* returns the fingerprint for the certificate */
    SSL* createSSL();                                                        /* creates a new SSL* object with support with DTLS, giving ownership to the caller. */

  private:
    bool createKey();                                                        /* creates a EVP_PKEY that is used to store private keys */
    bool createKeyAndCertificate();                                          /* creates a self signed certificate and private key */
    bool createCertificate();                                                /* creates the X509 certificate using EVP_PKEY member */
    bool createContext();                                                    /* creates the SSL_CTX instance; only after the certificate and key have been created. */
    bool loadPrivateKeyFile(std::string filepath);                           /* loads the private key from a file. */
    bool loadCertificateFile(std::string filepath);                          /* loads the certificate from a file. */

  public:
    X509* cert;                                                             /* the certificate */
    EVP_PKEY* pkey;                                                         /* the private key. */
    SSL_CTX* ctx;                                                           /* the SSL_CTX */
  };

} /* namespace dtls */

#endif
