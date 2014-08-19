#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <signaling/Signaling.h>

static void sigh(int sn);

sig::Signaling server;
sig::SignalingSettings cfg;

int main() {
  printf("\n\ntest_signaling\n\n");

  signal(SIGTERM, sigh);
  signal(SIGINT, sigh);

  cfg.port = "9001";

  if (server.init(cfg) < 0) {
    printf("main - error: cannot init the server.\n");
    exit(1);
  }

  const char* sdp = "sdp "
    "v=0\r\n"
    "o=- 5372151867866539221 2 IN IP4 127.0.0.1\r\n"
    "s=-\r\n"
    "c=IN IP4 192.168.0.193\r\n"
    "t=0 0\r\n"
    "a=ice-lite\r\n"
    "m=video 1 RTP/SAVPF 100\r\n"
    "a=rtpmap:100 VP8/90000\r\n"
    "a=mid:video\r\n"
    "a=recvonly\r\n"
    "a=rtcp-mux\r\n"
    "a=ice-ufrag:5PN2qmWqBl\r\n"
    "a=ice-pwd:Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC\r\n"
    "a=candidate:4252876256 1 udp 2122260223 192.168.0.193 59976 typ host\r\n"
    "a=candidate:4252876256 2 udp 2122260223 192.168.0.193 59976 typ host\r\n"
    "a=fingerprint:sha-256 3C:A8:D2:9B:34:9C:F1:94:F5:FD:AD:61:1D:79:21:4D:75:32:23:BB:ED:2E:85:02:79:C9:80:1D:A8:BB:A9:8A\r\n"
    "a=setup:passive\r\n";

  if (0 != server.addRoom(new sig::Room("party", sdp))) {
    printf("main - error: cannot add a room.\n");
    exit(1);
  }

  if (0 != server.start()) {
    printf("main - error: cannot start.\n");
    exit(1);
  }
  
  while(false == server.must_stop) {
    printf("still running ...\n");
    usleep(1000000);
  }

  return 0;
}


static void sigh(int sn) {
  signal(sn, sigh);  
  server.stop();
  printf("sigh - verbose: handling signal.\n");
}
