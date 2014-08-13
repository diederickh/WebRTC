#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <uv.h>
#include <ice/Agent.h>

namespace ice {

  /* ------------------------------------------------------------------ */

  /* gets called whenever the dtls connection needs to send some data back to the other party. */  
  static void agent_on_dtls_data(uint8_t* data, uint32_t nbytes, void* user);                   

  /* gets called whenever a stream receives data for a candidate pair that needs to be processed. */
  static void agent_stream_on_data(Stream* stream, 
                                   std::string rip, uint16_t rport,
                                   std::string lip, uint16_t lport,
                                   uint8_t* data, uint32_t nbytes, void* user);

  /* ------------------------------------------------------------------ */

  Agent::Agent() 
    :is_lite(true)
  {
    srand(time(NULL));
  }

  Agent::~Agent() {

    std::vector<Stream*>::iterator it = streams.begin();
    while(it != streams.end()) {
      delete *it;
      streams.erase(it);
    }

    /* @todo make sure the dtls::Parser is free'd (see stream data handler) */
  }

  /* Add a new stream; we take ownership */
  void Agent::addStream(Stream* stream) {

    if (!stream) {
      printf("ice::Agent - error: trying to add an invalid stream to an agent. Stream == null.\n");
      return;
    }

    streams.push_back(stream);

    stream->on_data = agent_stream_on_data;
    stream->user_data = this;
  }

  /* Initializes all the streams/candidates */
  bool Agent::init() {

    /* @todo - we're initializing the dtls::Context in Agent now, but this must be controlled by the user */
    if (!dtls_ctx.init("./server-cert.pem", "./server-key.pem")) {
      printf("ice::Agent - error: cannot initialize the dtls context.\n");
      return false;
    }

    /* and initialize all streams. */
    for (size_t i = 0; i < streams.size(); ++i) {
      if (!streams[i]->init()) {
        return false;
      }
    }

    return true;
  }

  /* Processes any incoming/outcoming data */
  void Agent::update() {
    for (size_t i = 0; i < streams.size(); ++i) {
      streams[i]->update();
    }
  }

  /* Set the ice-ufrag and ice-pwd values to use in stun messages (e.g. MessageIntegrity). */
  void Agent::setCredentials(std::string ufrag, std::string pwd) {

    if (0 == streams.size()) {
      printf("ice::Agent - warning: you're trying to set credentials but haven't added any streams yet.\n");
    }

    for (size_t i = 0; i < streams.size(); ++i) {
      streams[i]->setCredentials(ufrag, pwd);
    }
  }

  void Agent::handleStunMessage(Stream* stream, stun::Message* msg, 
                                std::string rip, uint16_t rport, 
                                std::string lip, uint16_t lport) 
  {

    /* Make sure we receive valid input. */
    if (!stream) {
      printf("ice::Agent::handleStunMesage() - error: cannot handle stun message, invalid stream given.\n");
      return;
    }

    if (!msg) {
      printf("ice::Agent::handleStunMesage() - error: cannot handle stun message because it's invalid.\n");
      return;
    }

    /* Handle the message */
    if (msg->type != stun::STUN_BINDING_REQUEST) {
      printf("ice::Agent::handleStunMesage() -  error: we only implement ice-controlled mode for now so we must receive a STUN_BINDING_REQUEST first.\n");
      return;
    }

    /* Find the local candidate that we use to transfer data from. */
    ice::Candidate* local_cand = stream->findLocalCandidate(lip, lport);
    if (!local_cand) {
      printf("ice::Agent::handleStunMessage() - error: cannot find the local candidate for %s:%u\n", lip.c_str(), lport);
      return;
    }

    /* Create the pair when the controlling agent tell us to use this candidate. */
    if (msg->hasAttribute(stun::STUN_ATTR_USE_CANDIDATE)) {
      CandidatePair* pair = stream->findPair(rip, rport, lip, lport);
      if (NULL == pair) {
        pair = stream->createPair(rip, rport, lip, lport);
      }
    }
    
    /* Construct our STUN Binding-Success-Response */
    stun::Message response(stun::STUN_BINDING_RESPONSE);
    response.copyTransactionID(msg);
    response.addAttribute(new stun::XorMappedAddress(rip, rport));
    response.addAttribute(new stun::MessageIntegrity());
    response.addAttribute(new stun::Fingerprint());

    /* Write + send the message. */
    stun::Writer writer;
    writer.writeMessage(&response, stream->ice_pwd);
    local_cand->conn.sendTo(rip, rport, &writer.buffer[0], writer.buffer.size());
  }

  void Agent::handleStreamData(Stream* stream, 
                               std::string rip, uint16_t rport, 
                               std::string lip, uint16_t lport, 
                               uint8_t* data, uint32_t nbytes)
  {

#if !defined(NDEBUG)
    if (NULL == on_rtp) {
      printf("Agent::handleStreamData() - error: no on_rtp() callback set; makes no sense to do anything with the data.\n");
      return;
    }
#endif

    /* Find a candidate pair. */
    CandidatePair* pair = stream->findPair(rip, rport, lip, lport);
    if (NULL == pair) {
      pair = stream->createPair(rip, rport, lip, lport);
      if (NULL == pair) {
        printf("Agent::handleStreamData() - error: cannot allocate a candidate pair!\n");
        return;
      }
    }

    Candidate* lcand = pair->local;

    /* INITIALIZE DTLS */
    /* --------------- */
    if (NULL == lcand->dtls.ssl) {
      lcand->dtls.on_data = agent_on_dtls_data;
      lcand->dtls.user = pair;

      /* Allocate our SSL* object. */
      lcand->dtls.ssl = dtls_ctx.createSSL();
      if (!lcand->dtls.ssl) {
          printf("agent_stream_on_data - error: cannot allocate a new SSL object.\n");
          exit(1);
      }
      if (!lcand->dtls.init()) {
        printf("agent_stream_on_data - error: cannot initialize the dtls parser.\n");
        exit(1);
      }
    }

    dtls::Parser& dtls = lcand->dtls;

    /* SETUP SRTP WHEN DTLS HANDSHAKE IS FINISHED */
    /* ------------------------------------------ */
    if (false == lcand->dtls.isHandshakeFinished()) {

      /* Handle data. */
      dtls.process(data, nbytes);

      /* When DTLS handshake is finished we can setup the SRTP flow */
      if (true == dtls.isHandshakeFinished()) {
        if (false == dtls.extractKeyingMaterial()) {
          printf("Agent::handleStreamData() - error: cannot extract keying material.\n");
          exit(1);
        }

        const char* cipher = dtls.getCipherSuite();
        if (NULL == cipher) {
          printf("Agent::handleStreamData() - error: cannot get cipher suite.\n");
          exit(1);
        }

        if (0 != lcand->srtp_in.init(cipher, true, dtls.remote_key, dtls.remote_salt)) {
          printf("Agent::handleStreamData() - erorr: cannot initialize srtp_in.\n");
          exit(1);
        }

        if (0 != lcand->srtp_out.init(cipher, false, dtls.local_key, dtls.local_salt)) {
          printf("Agent::handleStreamData() - erorr: cannot initialize srtp_out.\n");
          exit(1);
        }
      }
    }

    /* HANDLE MEDIA DATA */
    /* ----------------- */

    /* Ok, ready to decode some data with libsrtp. */
    /* @todo - distinguish between rtp/rtcp */
    int len = lcand->srtp_in.unprotectRTP(data, nbytes);
    if (len > 0) {
      if (stream->on_rtp) {
        stream->on_rtp(stream, pair, data, len, stream->user_rtp);
      }
    } 
    else {
      /* @todo we need to handle the srtp decoding error! */
    }
  }


  /* Experimantal API: returns the SDP */
  std::string Agent::getSDP() {
    std::string fingerprint;
    std::string sdp;
    std::stringstream ss;
    
    /* validate */
    if (0 == streams.size()) {
      printf("ice::Agent - error: cannot create the SDP because you haven't added a stream yet.\n");
      return sdp;
    }

    if (false == dtls_ctx.getFingerprint(fingerprint)) {
      printf("ice::Agent - error:cannot create the SDP, the DTLS context hasn't been initialized yet. Did you call init()?\n");
      return sdp;
    }

    /* session part */
    ss << "v=0\r\n"
       << "o=- " << uv_hrtime() << rand() << " 1 IN IP4 127.0.0.1\r\n"
       << "s=roxlu-webrtc\r\n"
       << "t=0 0\r\n"
       << "a=ice-lite\r\n";
        
    /* streams */
    for (size_t i = 0; i < streams.size(); ++i) {
      Stream* stream = streams[i];

      if ((stream->flags & STREAM_FLAG_VP8) == STREAM_FLAG_VP8) {
        ss << "m=video 1 RTP/SAVPF 100\r\n"
           << "c=IN IP4 127.0.0.1\r\n"
           << "a=rtpmap:100 VP8/90000\r\n";
      }

      if ((stream->flags & STREAM_FLAG_RTCP_MUX) == STREAM_FLAG_RTCP_MUX) {
        ss << "a=rtcp-mux\r\n";
      }

      if ((stream->flags & STREAM_FLAG_SENDRECV) == STREAM_FLAG_SENDRECV) {
        ss << "a=sendrecv\r\n";
      }
      else if ((stream->flags & STREAM_FLAG_RECVONLY) == STREAM_FLAG_RECVONLY) {
        ss << "a=recvonly\r\rn";
      }
      
      ss << "a=setup:passive\r\n";
      ss << "a=ice-ufrag:" << stream->ice_ufrag << "\r\n";
      ss << "a=ice-pwd:" << stream->ice_pwd << "\r\n";
      ss << "a=fingerprint:sha-256 " << fingerprint << "\r\n";
      
      for (size_t k = 0; k < stream->local_candidates.size(); ++k) {
        /* @todo - do we need two candidates when using rtcp-mux ? */
        Candidate* cand = stream->local_candidates[k];
        uint64_t foundation = uv_hrtime();
        ss << "a=candidate:" << foundation << " 1 udp 2130706431 " << cand->ip << " " << cand->port << " typ host\r\n";
        ss << "a=candidate:" << foundation << " 2 udp 2130706431 " << cand->ip << " " << cand->port << " typ host\r\n";
      }
    }

    sdp = ss.str();
    return sdp;
  }

  /* ------------------------------------------------------------------ */
  static void agent_stream_on_data(Stream* stream, 
                                   std::string rip, uint16_t rport,
                                   std::string lip, uint16_t lport,
                                   uint8_t* data, uint32_t nbytes, void* user) 
  {
    int r;
    stun::Message msg;
    ice::Agent* agent = static_cast<Agent*>(user);
    
    printf("agent_stream_on_data: verbose - received %u bytes from %s:%u on %s:%u\n", nbytes, rip.c_str(), rport, lip.c_str(), lport);

    /* process the incoming message, check if it's STUN, DTLS or RTP data */
    r = agent->stun.process(data, nbytes, &msg);
    if (r == 0) {
      /* STUN */
      agent->handleStunMessage(stream, &msg, rip, rport, lip, lport);
    }
    else if (r == 1) {
      /* RTP, RTCP or DTLS */
      agent->handleStreamData(stream, rip, rport, lip, lport, data, nbytes);
    }
  }

  static void agent_on_dtls_data(uint8_t* data, uint32_t nbytes, void* user) {

    ice::CandidatePair* pair = static_cast<ice::CandidatePair*>(user);

    if (NULL == pair->local) {
      printf("agent_on_dtls_data: error - the pair doesn't have a local candidate which isn't supposed to happen!\n");
      return;
    }

    pair->local->conn.sendTo(pair->remote->ip, pair->remote->port, data, nbytes);
  }                   

} /* namespace ice */

