#include <ice/Agent.h>

namespace ice {

  Agent::Agent() {
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
    streams.push_back(stream);
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
      for(size_t k = 0; k < stream->candidates.size(); ++k) {
        Candidate* cand = stream->candidates[k];
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

} /* namespace ice */

