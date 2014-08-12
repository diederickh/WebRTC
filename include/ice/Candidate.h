#ifndef ICE_CANDIDATE_H
#define ICE_CANDIDATE_H

#include <stdint.h>
#include <string>
#include <rtc/Connection.h>
#include <dtls/Context.h>
#include <dtls/Parser.h>
#include <srtp/ParserSRTP.h>

namespace ice {

  /* -------------------------------------------------- */

  class Candidate {
  public:
    Candidate(std::string ip, uint16_t port);
    bool init(connection_on_data_callback cb, void* user);            /* pass in the function which will receive the data from the socket. */
    void update();                                                    /* read data from the socket + process */

  public:
    std::string ip;                                                   /* the ip to which we can send data */ 
    uint16_t port;                                                    /* the port to which we can send data */
    uint8_t component_id;                                             /* compoment id */
    rtc::ConnectionUDP conn;                                          /* the (udp for now) connection on which we receive data; later we can decouple this if necessary. */
    dtls::Parser dtls;                                                /* used to handle the dtls handshake */
    connection_on_data_callback on_data;                              /* will be called whenever we receive data from the socket. */
    void* user;                                                       /* user data */
  };

  /* -------------------------------------------------- */

  class CandidatePair {
  public:
    CandidatePair();
    CandidatePair(Candidate* local, Candidate* remote);
    ~CandidatePair();

  public:
    Candidate* local;                                                 /* local candidate; which has a socket (ConnectionUDP) */
    Candidate* remote;                                                /* the remote party from which we receive data and send data towards. */
    dtls::Parser dtls;                                                /* each candidate pair has it's own dtls context; each pair has it's own 'flow' of data */
    srtp::ParserSRTP srtp_out;                                        /* used to protect outgoing data. */
    srtp::ParserSRTP srtp_in;                                         /* used to unprotect incoming data. */
  };

} /* namespace ice */

#endif
