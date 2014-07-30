#include <rtc/Connection.h>

/* ----------------------------------------------------------------- */

static void rtc_connection_udp_alloc_cb(uv_handle_t* handle, size_t nsize, uv_buf_t* buf);
static void rtc_connection_udp_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags);
static void rtc_connection_udp_send_cb(uv_udp_send_t* req, int status);

/* ----------------------------------------------------------------- */

namespace rtc {
  
  ConnectionUDP::ConnectionUDP() 
    :loop(NULL)
    ,user(NULL)
    ,on_data(NULL)
  {

    loop = uv_default_loop();
    if (!loop) {
      printf("Error: ConnectionUDP() cannot get the default uv loop.\n");
      ::exit(1);
    }

  }

  bool ConnectionUDP::bind(std::string ip, uint16_t port) {

    int r;

    /* create sockaddr */
    r = uv_ip4_addr(ip.c_str(), port, &addr);
    if (r != 0) {
      printf("Error: cannot create sockaddr_in: %s\n", uv_strerror(r));
      return false;
    }

    /* initialize the socket */
    r = uv_udp_init(loop, &sock);
    if (r != 0) {
      printf("Error: cannot initialize the UDP socket in ConnectionUDP: %s\n", uv_strerror(r));
      return false;
    }

    /* bind */
    r  = uv_udp_bind(&sock, (const struct sockaddr*)&addr, 0);
    if (r != 0) {
      printf("Error: cannot bind the UDP socket in ConnectionUDP: %s\n", uv_strerror(r));
      return false;
    }

    sock.data = (void*) this;

    /* start receiving */
    r = uv_udp_recv_start(&sock, 
                          rtc_connection_udp_alloc_cb,
                          rtc_connection_udp_recv_cb);

    if (r != 0) {
      printf("Error: cannot start receiving in ConnectionUDP: %s\n", uv_strerror(r));
      return false;
    }

    return true;
  }

  void ConnectionUDP::send(uint8_t* data, uint32_t nbytes) {

    uv_udp_send_t* req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
    if (!req) {
      printf("Error: cannot allocate a send request in ConnectionUDP.\n");
      return;
    }

    /* @todo check nbytes size in ConnectionUDP::send */
    /* @todo we def. don't want to allocate everytime when we need to sentin ConnectionUDP. */

    char* buffer_copy = new char[nbytes];
    if (!buffer_copy) {
      printf("Error: cannot allocate a copy for the send buffer in ConnectionUDP.\n");
      free(req);
      req = NULL;
      return;
    }

    memcpy(buffer_copy, data, nbytes);
    uv_buf_t buf = uv_buf_init(buffer_copy, nbytes);
    
    req->data = buffer_copy;

    int r = uv_udp_send(req, 
                        &sock, 
                        &buf, 
                        1, 
                        (const struct sockaddr*)&addr, 
                        rtc_connection_udp_send_cb);

    if (r != 0) {
      printf("Error: cannot send udp data in ConnectionUDP: %s.\n", uv_strerror(r));
      free(req);
      free(buffer_copy);
      req = NULL;
      buffer_copy = NULL;
    }
  }

  void ConnectionUDP::update() {
    uv_run(loop, UV_RUN_NOWAIT);
  }

} /* namespace rtc */

/* ----------------------------------------------------------------- */

static void rtc_connection_udp_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags) {
  rtc::ConnectionUDP* udp = static_cast<rtc::ConnectionUDP*>(handle->data);
  if (udp->on_data) {
    udp->on_data((uint8_t*)buf->base, nread, udp->user);
  }
}

static void rtc_connection_udp_alloc_cb(uv_handle_t* handle, size_t nsize,  uv_buf_t* buf) {
  static char slab[65536];
 
  if (nsize > sizeof(slab)) {
    printf("Error: requested receiver size to large. @todo - this is just a quick implementation.\n");
    exit(1);
  }
  
  buf->base = slab;
  buf->len = sizeof(slab);
}

static void rtc_connection_udp_send_cb(uv_udp_send_t* req, int status) {
  /* @todo rtc_connection_udp_send_cb needs to handle the status value.*/
  char* ptr = (char*)req->data;
  delete[] ptr;
  delete req;
  req = NULL;
  ptr = NULL;
}
