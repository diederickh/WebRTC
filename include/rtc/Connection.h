/*

  Connection
  -----------
  This code is still experimental and the API will/may change. At this 
  moment I'm not sure how to abstract the network I/O (udp/tcp) and the 
  coupling with DTLS. 

  At this moment there is a base Connection and an ConnectionUDP class.

*/
#ifndef RTC_CONNECTION_H
#define RTC_CONNECTION_H

extern "C" {
#  include <uv.h>  
}

#include <stdlib.h>
#include <stdint.h>
#include <string>

typedef void(*connection_on_data_callback)(uint8_t* data, uint32_t nbytes, void* user);  /* gets called when a connection receives some data. */

namespace rtc {

  class Connnection {
  };

  class ConnectionUDP {

  public:
    ConnectionUDP();
    bool bind(std::string ip, uint16_t port);
    void update();
    

  public:
    struct sockaddr_in addr;
    uv_udp_t sock;
    uv_loop_t* loop;

    /* callbacks */
    connection_on_data_callback on_data;
    void* user;
  };

} /* namespace rtc */

#endif
