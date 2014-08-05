#ifndef ICE_CANDIDATE_H
#define ICE_CANDIDATE_H

#include <stdint.h>
#include <string>
#include <rtc/Connection.h>
#include <stun/Reader.h>
#include <dtls/Parser.h>

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

  class Candidate {
  public:
    Candidate(std::string ip, uint16_t port);
    bool init();
    void update();                                                    /* read data from the socket + process */
    void setCredentials(std::string ufrag, std::string pwd);          /* set the ice-ufrag and ice-pwd values. */

  public:
    std::string ip;
    uint16_t port;
    std::string ice_ufrag;                                            /* ice-ufrag value */
    std::string ice_pwd;                                              /* ice-pwd value */ 
    CandidateState state;                                             
    rtc::ConnectionUDP conn;                                          /* the (udp for now) connection on which we receive data; later we can decouple this if necessary. */
    stun::Reader stun;                                                /* the stun reader; used to parse incoming stun messages */
    dtls::Parser dtls;                                                /* used to handle the dtls handshake */
  };


} /* namespace ice */
#endif
