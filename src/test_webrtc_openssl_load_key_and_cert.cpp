#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

int main() {
  FILE* fp;
  EVP_PKEY* pkey;
  X509* cert;

  printf("\n\ntest_openssl_load_key_and_cert\n\n");

  /* init ssl */
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
    pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    if (!pkey) {
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
    
    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    if (!cert) {
      printf("Error: cannot read certificate.\n");
      exit(1);
    }
    
    fclose(fp);
    fp = NULL;
  }

  printf("Loaded certificate.\n");

  X509_free(cert);

  ERR_print_errors_fp(stdout);

  return 0;
}
