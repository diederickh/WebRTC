#include <ice/Stream.h>

namespace ice {

  /* ------------------------------------------------------------------ */

  /* gets called when a candidate receives data. */
  static void stream_on_data(std::string rip, uint16_t rport, std::string lip, uint16_t lport, uint8_t* data, uint32_t nbytes, void* user);  

  /* ------------------------------------------------------------------ */

  Stream::Stream() 
    :state(STREAM_STATE_NONE)
    ,on_data(NULL)
    ,user_data(NULL)
    ,on_rtp(NULL)
    ,user_rtp(NULL)
  {

  }

  Stream::~Stream() {

    {
      /* local candidates */
      std::vector<Candidate*>::iterator it = local_candidates.begin();
      while (it != local_candidates.end()) {
        delete *it;
        it = local_candidates.erase(it);
      }
    }

    {
      /* remote candidates */
      std::vector<Candidate*>::iterator it = remote_candidates.begin();
      while (it != remote_candidates.end()) {
        delete *it;
        it = remote_candidates.erase(it);
      }
    }

    {
      /* candidate pairs */
      std::vector<CandidatePair*>::iterator it = pairs.begin();
      while (it != pairs.end()) {
        delete *it;
        it = pairs.erase(it);
      }
    }
  }

  bool Stream::init() {

    for (size_t i = 0; i < local_candidates.size(); ++i) {
      if (!local_candidates[i]->init(stream_on_data, this)) {
        return false;
      }
    }

    return true;
  }

  void Stream::update() {
    for (size_t i = 0; i < local_candidates.size(); ++i) {
      local_candidates[i]->update();
    }
  }

  void Stream::addLocalCandidate(Candidate* c) {
    local_candidates.push_back(c);
  }

  void Stream::addRemoteCandidate(Candidate* c) {
    remote_candidates.push_back(c);
  }

  void Stream::addCandidatePair(CandidatePair* p) {
    pairs.push_back(p);
  }

  void Stream::setCredentials(std::string ufrag, std::string pwd) {
    for (size_t i = 0; i < local_candidates.size(); ++i) {
      local_candidates[i]->setCredentials(ufrag, pwd);
    }
  }

  CandidatePair* Stream::findPair(std::string rip, uint16_t rport, std::string lip, uint16_t lport) {

    if (pairs.size() == 0) {
      return NULL;
    }

    for (size_t i = 0; i < pairs.size(); ++i) {
      CandidatePair* p = pairs[i];
      if (p->local->port != lport) {
        continue;
      }
      if (p->remote->port != rport) {
        continue;
      }
      if (p->local->ip != lip) {
        continue;
      }
      if (p->remote->ip != rip) {
        continue;
      }
      return p;
    }

    return NULL;
  }

  Candidate* Stream::findLocalCandidate(std::string ip, uint16_t port) {
    for (size_t i = 0; i < local_candidates.size(); ++i) {
      Candidate* c = local_candidates[i];
      if (c->port != port) {
        continue;
      }
      if (c->ip != ip) {
        continue;
      }
      return c;
    }
    return NULL;
  }

  Candidate* Stream::findRemoteCandidate(std::string ip, uint16_t port) {
    for (size_t i = 0; i < remote_candidates.size(); ++i) {
      Candidate* c = remote_candidates[i];
      if (c->port != port) {
        continue;
      }
      if (c->ip != ip) {
        continue;
      }
      return c;
    }
    return NULL;
  }

  /* ------------------------------------------------------------------ */

  /* 
     This is called whenever a candidate receives data. Each stream (e.g. video, audio, or muxed),
     will check if there is an existing candidate pair for the transport. If it doesn't exist
     it will create a new one. 

     It may happen that a candidate pair is created but never used because another pair is selected.
     At this moment the candidate pair is not free'd and simply ignored.

     @todo - stream_on_data, implement candidate/candidate-pair states, so we can free unused pairs.

   */
  static void stream_on_data(std::string rip, uint16_t rport, 
                             std::string lip, uint16_t lport, 
                             uint8_t* data, uint32_t nbytes, void* user) 
  {
    printf("stream_on_data - verbose: %s:%u ---> %s:%u\n", rip.c_str(), rport, lip.c_str(), lport);

    Stream* stream = static_cast<Stream*>(user);
    CandidatePair* pair = stream->findPair(rip, rport, lip, lport);

    if (!pair) {

      Candidate* local_candidate = stream->findLocalCandidate(lip, lport);
      if (!local_candidate) {
        printf("stream_on_data - verbose: cannot find a local candidate for: %s:%u\n", lip.c_str(), lport);
        return;
      }

      Candidate* remote_candidate = stream->findRemoteCandidate(rip, rport);
      if (remote_candidate) {
        printf("stream_on_data - error: we found a remote candidate for which no pair exists; this shouldn't happen!\n");
        exit(1);
      }

      /* create the new remote candidate */
      remote_candidate = new Candidate(rip, rport);
      stream->addRemoteCandidate(remote_candidate);
      
      /* and create the pair to keep track of ice state! */
      pair = new CandidatePair(local_candidate, remote_candidate);
      stream->addCandidatePair(pair);
    }

    if (!pair) {
      printf("stream_on_data - error: we should have found or created a candidate pair by now!\n");
      exit(1);
    }

    if (stream->on_data) {
      stream->on_data(stream, pair, data, nbytes, stream->user_data);
    }
  }

} /* namespace ice */

