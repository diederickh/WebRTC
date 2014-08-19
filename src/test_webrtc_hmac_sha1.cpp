#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

int main() {
  printf("\n\ntest_hmac_sha1\n\n");

  std::string data = "Lorem ipsum dolor sit amet, consectetuer"
                              "adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum"
                              "sociis natoque penatibus et magnis dis parturient montes, nascetur"
                              "ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu,"
                              "pretium quis, sem. Nulla consequat massa quis enim. Donec pede"
                              "justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim"
                              "justo, rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam"
                              "dictum felis eu pede mollis pretium. Integer tincidunt. Cras"
                              "dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend"
                              "tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend"
                              "ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a,"
                              "tellus. Phasellus viverra nulla ut metus varius laoreet. Quisque"
                              "rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue. Curabitur"
                              "ullamcorper ultricies nisi. Nam eget dui.";

  std::string key = "z2L4bezUSUjQUqSAJBvnMxza";
  unsigned char result[20]; 
  unsigned int len;
  HMAC_CTX ctx;

  HMAC_CTX_init(&ctx);
  
  if (!HMAC_Init_ex(&ctx, (const unsigned char*)key.c_str(), key.size(), EVP_sha1(), NULL)) {
    printf("Error: cannot init the HMAC.\n");
  }

  HMAC_Update(&ctx, (const unsigned char*)data.c_str(), data.size());
  HMAC_Final(&ctx, result, &len);

  printf("Hash: ");
  for(unsigned int i = 0; i < len; ++i) {
    printf("%02X ", result[i]);
  }
  printf("\n\n");
}
