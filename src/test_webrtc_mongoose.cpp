#include <net_skeleton.h>
#include <ssl_wrapper.h>
#include <mongoose.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static int ev_handler(struct mg_connection* conn, enum mg_event ev);
static void *serve_thread_func(void *param);
static int iterate_callback(struct mg_connection *c, enum mg_event ev);

int main() {

  printf("\n\ntest_mongoose\n\n");

  struct mg_server* server = mg_create_server(NULL, ev_handler);
  mg_set_option(server, "listening_port", "9001");
  mg_start_thread(serve_thread_func, server);

  while(1) {
  }

  return 0;
}

static void *serve_thread_func(void *param) {
  struct mg_server *server = (struct mg_server *) param;
  while (1) {
    mg_poll_server(server, 100);
  }
  mg_destroy_server(&server);
  return NULL;
}

static int ev_handler(struct mg_connection* conn, enum mg_event ev) {

  if (ev == MG_REQUEST) {
    if (conn->is_websocket) {
      if (conn->content_len >= 6 && 0 == memcmp(conn->content, "getsdp", 6)) {
        printf("get SDP!\n");

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

        mg_websocket_write(conn, 1, sdp, strlen(sdp) + 1);
      }
      return conn->content_len == 4 && !memcmp(conn->content, "exit", 4) ? MG_FALSE : MG_TRUE;
    }
  } 
  else if (ev == MG_AUTH) {
    return MG_TRUE;
  } 
  return MG_FALSE;
}


