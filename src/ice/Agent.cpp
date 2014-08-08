#include <ice/Agent.h>

namespace ice {

  /* ------------------------------------------------------------------ */

  /* gets called whenever a stream receives data for a candidate pair that needs to be processed. */
  static void agent_stream_on_data(Stream* stream, CandidatePair* pair, uint8_t* data, uint32_t nbytes, void* user);

  /* ------------------------------------------------------------------ */

  Agent::Agent() 
    :is_lite(true)
  {
  }

  Agent::~Agent() {

    std::vector<Stream*>::iterator it = streams.begin();
    while(it != streams.end()) {
      delete *it;
      streams.erase(it);
    }
    
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

    /* set the parser.ssl for each of the stream candidates, @todo there must be a better API for this. */
    for(size_t i = 0; i < streams.size(); ++i) {
      Stream* stream = streams[i];

      /* @todo - we're going to remove the dtls from candidate; instead CandidatePair gets one. */
      for(size_t k = 0; k < stream->local_candidates.size(); ++k) {
        Candidate* cand = stream->local_candidates[k];
        cand->dtls.ssl = dtls_ctx.createSSL();
        if (!cand->dtls.ssl) {
          printf("ice::Agent - error: cannot create the SSL* for the candidate.\n");
          return false;
        }
      }
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

  void Agent::handleStunMessage(Stream* stream, CandidatePair* pair, stun::Message* msg) {

    /* Make sure we receive valid input. */
    if (!stream) {
      printf("ice::Agent::handleStunMesage() - error: cannot handle stun message, invalid stream given.\n");
      return;
    }

    if (!pair) {
      printf("ice::Agent::handleStunMesage() - error: cannot handle stun message, invald pair given.\n");
      return;
    }

    if (!pair->local) {
      printf("ice::Agent::handleStunMesage() - error: the pair doesn't have a local candidate, which isn't allowed.\n");
      return;
    }

    if (!pair->remote) {
      printf("ice::Agent::handleStunMesage() - error: the pair doesn't have a remote candidate, which isn't allowed.\n");
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

    /* Construct our STUN Binding-Success-Response */
    stun::Message response(stun::STUN_BINDING_RESPONSE);
    response.copyTransactionID(msg);
    response.addAttribute(new stun::XorMappedAddress(pair->remote->ip, pair->remote->port));
    response.addAttribute(new stun::MessageIntegrity());
    response.addAttribute(new stun::Fingerprint());

    /* Write + send the message. */
    stun::Writer writer;
    writer.writeMessage(&response, pair->local->ice_pwd);
    pair->local->conn.sendTo(pair->remote->ip, pair->remote->port,
                             &writer.buffer[0], writer.buffer.size());

  }

  /* ------------------------------------------------------------------ */

  static void agent_stream_on_data(Stream* stream, CandidatePair* pair, uint8_t* data, uint32_t nbytes, void* user) {
    printf("agent_stream_on_data - verbose: received data form a stream.\n");

    int r;
    stun::Message msg;
    Agent* agent = static_cast<Agent*>(user);

    r = agent->stun.process(data, nbytes, &msg);
    if (r == 0) {
      agent->handleStunMessage(stream, pair, &msg);
    }
    else if (r == 1) {

      /* Not SSL context yet, create it! */
      if (pair->dtls.ssl == NULL) {
        pair->dtls.ssl = agent->dtls_ctx.createSSL();
        if (!pair->dtls.ssl) {
          printf("agent_stream_on_data - error: cannot allocate a new SSL object.\n");
          exit(1);
        }
        if (!pair->dtls.init()) {
          printf("agent_stream_on_data - error: cannot initialize the dtls parser.\n");
          exit(1);
        }
      }
      
      if (!pair->dtls.isHandshakeFinished()) {
        /* Process the 'non' stun data, which should be DTLS */
        pair->dtls.process(data, nbytes);
      }
      else {

        /* @todo - okay we need to code this whole function way better ^.^ */
        if (pair->dtls.mode == dtls::DTLS_MODE_SERVER) {
          if (pair->srtp_reader.is_initialized == false) {

            /* After processing the data, check if the handshake is finished and if so, use the extracted key/salts for SRTP. */
            if (!pair->dtls.extractKeyingMaterial()) {
              printf("agent_stream_on_data - error: cannot extract keying material.\n");
              exit(1);
            }

            //if (!pair->srtp_reader.init(pair->dtls.client_key, pair->dtls.client_salt)) {
            if (!pair->srtp_reader.init(pair->dtls.remote_key, pair->dtls.remote_salt)) {
              printf("agent_stream_on_data - error: cannot init the srtp reader.\n");
              exit(1);
            }
          }
        }
        else {
          printf("agent_stream_on_data - error: only implmenting the DTLS_MODE_SERVER for now.\n");
          exit(1);
        }
        
        /* Handshake ready, so here the SRTP reader should be ready as well!. */
        if (false == pair->srtp_reader.is_initialized) {
          printf("agent_stream_on_data - error: dtls handshake done, but srtp reader not initialized - not supposed to happen!\n");
          exit(1);
        }

        /* Ok, ready to decode some data with libsrtp. */
        pair->srtp_reader.process(data, nbytes);
        if (stream->on_rtp) {
          stream->on_rtp(stream, pair, data, nbytes, stream->user_rtp);
        }
      }
    }
  }

} /* namespace ice */

