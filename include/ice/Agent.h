/*
  
  Agent
  -----

  This is an experimental class to keep track of the state of an agent 
  and it's candidates. 

  Should be used with a (server) sdp, with a=ice-lite, e.g:

  <example>
      v=0
      o=- 5372151867866539221 2 IN IP4 127.0.0.1
      s=-
      t=0 0
      a=ice-lite
      m=video 1 RTP/SAVPF 100
      c=IN IP4 192.168.0.193
      a=mid:video
      a=recvonly
      a=rtcp-mux
      a=rtpmap:100 VP8/90000
      a=ice-ufrag:5PN2qmWqBl
      a=ice-pwd:Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC
      a=candidate:4252876256 1 udp 2122260223 192.168.0.193 59976 typ host
      a=candidate:4252876256 2 udp 2122260223 192.168.0.193 59976 typ host
      a=fingerprint:sha-256 3C:A8:D2:9B:34:9C:F1:94:F5:FD:AD:61:1D:79:21:4D:75:32:23:BB:ED:2E:85:02:79:C9:80:1D:A8:BB:A9:8A
      a=setup:passive
  </example>


  References:
  -----------
  - Agent states: http://docs.webplatform.org/wiki/apis/webrtc/RTCPeerConnection/iceState

 */

#ifndef ICE_AGENT_H
#define ICE_AGENT_H

#include <string>
#include <vector>
#include <ice/Types.h>
#include <ice/Stream.h>
#include <dtls/Context.h>
#include <stun/Reader.h>
#include <stun/Writer.h>

namespace ice {


  enum AgentState {
    AGENT_STATE_NONE = 0x0,
    AGENT_STATE_STARTING,                   
    AGENT_STATE_CHECKING, 
    AGENT_STATE_CONNECTED,
    AGENT_STATE_COMPLETED,
    AGENT_STATE_FAILED,
    AGENT_STATE_DISCONNECTED,
    AGENT_STATE_CLOSED
  };

  class Agent {
  public:
    Agent();
    ~Agent();
    bool init();                                                                           /* After adding streams (and candidates to streams), call init to kick off everythign */
    void update();                                                                         /* This must be called often as it fetches new data from the socket and parses any incoming data */
    void addStream(Stream* stream);                                                        /* Add a new stream, this class takes ownership */
    void setCredentials(std::string ufrag, std::string pwd);                               /* set the credentials (ice-ufrag, ice-pwd) for all streams. */

    /* ICE */
    void handleStunMessage(Stream* stream, CandidatePair* pair, stun::Message* msg);       /* Handles incoming stun messages for the given stream and candidates. It will make sure the correct action will be taken. */

  public:
    std::vector<Stream*> streams;         
    dtls::Context dtls_ctx;                                                                /* The dtls::Context is used to handle the dtls communication */
    stun::Reader stun;                                                                     /* Used to parse incoming data and detect stun messages */
    bool is_lite;                                                                          /* At this moment we only support ice-lite. */
  };
} /* namespace ice */

#endif
