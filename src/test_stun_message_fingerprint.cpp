#include <stdio.h>
#include <stun/Utils.h>
#include <stun/Reader.h>
#include <stun/Writer.h>

#define PASSWD "VOkJxbRl1RmTxUk/WvJxBt"
static void on_stun_message(stun::Message* msg, void* user);

int main() {
  printf("\n\ntest_stun_message_fingerprint\n\n");

 const unsigned char req[] =
   "\x00\x01\x00\x58"
   "\x21\x12\xa4\x42"
   "\xb7\xe7\xa7\x01\xbc\x34\xd6\x86\xfa\x87\xdf\xae"
   "\x80\x22\x00\x10"
   "STUN test client"
   "\x00\x24\x00\x04"
   "\x6e\x00\x01\xff"
   "\x80\x29\x00\x08"
   "\x93\x2f\xf9\xb1\x51\x26\x3b\x36"
   "\x00\x06\x00\x09"
   "\x65\x76\x74\x6a\x3a\x68\x36\x76\x59\x20\x20\x20"
   "\x00\x08\x00\x14"
   "\x9a\xea\xa7\x0c\xbf\xd8\xcb\x56\x78\x1e\xf2\xb5"
   "\xb2\xd3\xf2\x49\xc1\xb5\x71\xa2"
   "\x80\x28\x00\x04"
   "\xe5\x7a\x3b\xcf";

 int nl = 0;
 printf("\nINPUT");
 printf("\n-----------\n");
 for (int i = 0; i < sizeof(req) - 1; ++i, ++nl) {

   if (nl == 4) {
     printf("\n");
     nl=0;
   }
   printf("%02X ", req[i]);
 }
 printf("\n-----------\n\n");


 stun::Reader reader;
 reader.on_message = on_stun_message;
 reader.process((uint8_t*)req, sizeof(req) - 1);
 
 return 0;
}

static void on_stun_message(stun::Message* msg, void* user) {
  printf("Received a stun message\n");
  
  /* 
     We write the message and calculate the message integrity + fingerprint.
     Note, because the padded bytes of the source contain 0x20 values and we
     use 0x00, our message integrity value and fingerprint value are different. 
  */
  stun::Writer writer;
  writer.writeMessage(msg, PASSWD);

 int nl = 0;
 printf("\nOUTPUT");
 printf("\n-----------\n");
 for (int i = 0; i < writer.buffer.size(); ++i, ++nl) {

   if (nl == 4) {
     printf("\n");
     nl=0;
   }
   printf("%02X ", writer.buffer[i]);
 }

 printf("\n-----------\n\n");
  
}
