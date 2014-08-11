#ifndef ICE_CANDIDATE_H
#define ICE_CANDIDATE_H

#include <stdint.h>
#include <string>
#include <rtc/Connection.h>
#include <stun/Reader.h>
#include <dtls/Parser.h>
#include <dtls/Context.h>
#include <srtp/Reader.h>
#include <srtp/ContextSRTP.h>

namespace ice {

  /* from: http://tools.ietf.org/html/rfc5245#section-5.7.4 */
  enum CandidateState {
    CANDIDATE_STATE_NONE = 0x00,
    CANDIDATE_STATE_WAITING, 
    CANDIDATE_STATE_IN_PROGRESS,
    CANDIDATE_STATE_SUCCEEDED,
    CANDIDATE_STATE_FAILED, 
    CANDIDATE_STATE_FROZEN
  };

  /* -------------------------------------------------- */

  class Candidate {
  public:
    Candidate(std::string ip, uint16_t port);
    bool init(connection_on_data_callback cb, void* user);            /* pass in the function which will receive the data from the socket. */
    void update();                                                    /* read data from the socket + process */
    void setCredentials(std::string ufrag, std::string pwd);          /* set the ice-ufrag and ice-pwd values. */

  public:
    CandidateState state;                                             /* candidate state; used by ice */
    std::string ip;                                                   /* the ip to which we can send data */ 
    uint16_t port;                                                    /* the port to which we can send data */
    std::string ice_ufrag;                                            /* ice-ufrag value */
    std::string ice_pwd;                                              /* ice-pwd value, used to construct message-integrity attributes */ 
    rtc::ConnectionUDP conn;                                          /* the (udp for now) connection on which we receive data; later we can decouple this if necessary. */
    stun::Reader stun;                                                /* the stun reader; used to parse incoming stun messages */
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
    srtp::Reader srtp_reader;                                         /* used to decode incoming SRTP packets. */
    srtp::ContextSRTP srtp_out;
  };

} /* namespace ice */

#endif
