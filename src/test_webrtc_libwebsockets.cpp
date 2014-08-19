/*

  test_libwebsockets
  ------------------

  Unfinished test with libwebsockets; at some point libwebrtc may implement a simple
  signaling server to add support for easy interop with a web app.

 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <libwebsockets.h>

#define RTC_SIGNAL_MSG_SIZE 1024
#define RTC_SIGNAL_BUFFER_SIZE (LWS_SEND_BUFFER_PRE_PADDING + RTC_SIGNAL_MSG_SIZE + LWS_SEND_BUFFER_POST_PADDING)


static int rtc_signaling_callback(struct libwebsocket_context* ctx, 
                                  struct libwebsocket* wsi, 
                                  enum libwebsocket_callback_reasons reason, 
                                  void* user, void* in, size_t len);

typedef struct rtc_signal_data rtc_signal_data;

struct rtc_signal_data {
  uint8_t buffer[RTC_SIGNAL_BUFFER_SIZE];
  uint32_t nbytes;
  uint32_t dx;
};

struct libwebsocket_protocols protos[] = {
  { "signaling", rtc_signaling_callback, sizeof(rtc_signal_data) },
  { NULL, NULL, 0 }
};

int main() {

  printf("\n\ntest_libwebsockets\n\n");

  struct libwebsocket_context* ctx;
  struct lws_context_creation_info info;
  struct libwebsocket* wsi;

  memset(&info, 0x00, sizeof(info));

  lwsl_notice("Created.\n");

  info.port = 55889;
  info.iface = NULL;
  info.protocols = protos;
  info.extensions = libwebsocket_get_internal_extensions();
  info.gid = -1;
  info.uid = -1;

  ctx = libwebsocket_create_context(&info);
  if (!ctx) {
    printf("Error: cannot create libwebsocket context.\n");
    exit(1);
  }

  int n = 0;
  while(n >= 0) {
    n = libwebsocket_service(ctx, 10);
  }
  
  return 0;
}

static int rtc_signaling_callback(struct libwebsocket_context* ctx, 
                              struct libwebsocket* wsi, 
                              enum libwebsocket_callback_reasons reason, 
                              void* user, void* in, size_t len)
{
  printf("Received callback.\n");
  switch(reason) {
    case LWS_CALLBACK_ESTABLISHED: { printf("LWS_CALLBACK_ESTABLISHED\n"); break; } 
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: { printf("LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n"); break; } 
    case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: { printf("LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH\n"); break; } 
    case LWS_CALLBACK_CLIENT_ESTABLISHED: { printf("LWS_CALLBACK_CLIENT_ESTABLISHED\n"); break; } 
    case LWS_CALLBACK_CLOSED: { printf("LWS_CALLBACK_CLOSED\n"); break; } 
    case LWS_CALLBACK_CLOSED_HTTP: { printf("LWS_CALLBACK_CLOSED_HTTP\n"); break; } 
    case LWS_CALLBACK_RECEIVE: { printf("LWS_CALLBACK_RECEIVE\n"); break; } 
    case LWS_CALLBACK_CLIENT_RECEIVE: { printf("LWS_CALLBACK_CLIENT_RECEIVE\n"); break; } 
    case LWS_CALLBACK_CLIENT_RECEIVE_PONG: { printf("LWS_CALLBACK_CLIENT_RECEIVE_PONG\n"); break; } 
    case LWS_CALLBACK_CLIENT_WRITEABLE: { printf("LWS_CALLBACK_CLIENT_WRITEABLE\n"); break; } 
    case LWS_CALLBACK_SERVER_WRITEABLE: { printf("LWS_CALLBACK_SERVER_WRITEABLE\n"); break; } 
    case LWS_CALLBACK_HTTP: { printf("LWS_CALLBACK_HTTP\n"); break; } 
    case LWS_CALLBACK_HTTP_BODY: { printf("LWS_CALLBACK_HTTP_BODY\n"); break; } 
    case LWS_CALLBACK_HTTP_BODY_COMPLETION: { printf("LWS_CALLBACK_HTTP_BODY_COMPLETION\n"); break; } 
    case LWS_CALLBACK_HTTP_FILE_COMPLETION: { printf("LWS_CALLBACK_HTTP_FILE_COMPLETION\n"); break; } 
    case LWS_CALLBACK_HTTP_WRITEABLE: { printf("LWS_CALLBACK_HTTP_WRITEABLE\n"); break; } 
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION: { printf("LWS_CALLBACK_FILTER_NETWORK_CONNECTION\n"); break; } 
    case LWS_CALLBACK_FILTER_HTTP_CONNECTION: { printf("LWS_CALLBACK_FILTER_HTTP_CONNECTION\n"); break; } 
    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED: { printf("LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED\n"); break; } 
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: { printf("LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION\n"); break; } 
    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: { printf("LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS\n"); break; } 
    case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS: { printf("LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS\n"); break; } 
    case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION: { printf("LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION\n"); break; } 
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: { printf("LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER\n"); break; } 
    case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY: { printf("LWS_CALLBACK_CONFIRM_EXTENSION_OKAY\n"); break; } 
    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED: { printf("LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED\n"); break; } 
    case LWS_CALLBACK_PROTOCOL_INIT: { printf("LWS_CALLBACK_PROTOCOL_INIT\n"); break; } 
    case LWS_CALLBACK_PROTOCOL_DESTROY: { printf("LWS_CALLBACK_PROTOCOL_DESTROY\n"); break; } 
    case LWS_CALLBACK_WSI_CREATE: { printf("LWS_CALLBACK_WSI_CREATE\n"); break; } 
    case LWS_CALLBACK_WSI_DESTROY: { printf("LWS_CALLBACK_WSI_DESTROY\n"); break; } 
    case LWS_CALLBACK_GET_THREAD_ID: { printf("LWS_CALLBACK_GET_THREAD_ID\n"); break; } 

    case LWS_CALLBACK_ADD_POLL_FD: { printf("LWS_CALLBACK_ADD_POLL_FD\n"); break; } 
    case LWS_CALLBACK_DEL_POLL_FD: { printf("LWS_CALLBACK_DEL_POLL_FD\n"); break; } 
    case LWS_CALLBACK_CHANGE_MODE_POLL_FD: { printf("LWS_CALLBACK_CHANGE_MODE_POLL_FD\n"); break; } 
    case LWS_CALLBACK_LOCK_POLL: { printf("LWS_CALLBACK_LOCK_POLL\n"); break; } 
    case LWS_CALLBACK_UNLOCK_POLL: { printf("LWS_CALLBACK_UNLOCK_POLL\n"); break; } 

    default: { 
      printf("callback - unhandled.\n");
      break;
    }
  }
  return 0;
}
