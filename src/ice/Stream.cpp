#include <ice/Stream.h>

namespace ice {

  /* ------------------------------------------------------------------ */

  /* gets called when a candidate receives data. */
  static void stream_on_data(std::string rip, uint16_t rport, std::string lip, uint16_t lport, uint8_t* data, uint32_t nbytes, void* user);  

  /* ------------------------------------------------------------------ */

  Stream::Stream(uint32_t flags) 
    :on_data(NULL)
    ,user_data(NULL)
    ,on_rtp(NULL)
    ,user_rtp(NULL)
    ,flags(flags)
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
    ice_ufrag = ufrag;
    ice_pwd = pwd;
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

  CandidatePair* Stream::createPair(std::string rip, uint16_t rport, std::string lip, uint16_t lport) {
    ice::Candidate* remote_cand = NULL;
    ice::Candidate* local_cand = NULL;
    ice::CandidatePair* pair = NULL;

    /* We shouldn't find this pair */
    pair = findPair(rip, rport, lip, lport);
    if (NULL != pair) {
      printf("ice::Stream::createPair() - pair already exists. %s:%u <-> %s:%u\n", lip.c_str(), lport, rip.c_str(), rport);
      return pair;
    }

    local_cand = findLocalCandidate(lip, lport);
    if (NULL == local_cand) {
      printf("ice::Stream::createPair() - error: cannot find a local candidate; we can only create candidate when the local one has been added alread. (e.g. by the calling app.).\n");
      return NULL;
    }

    /* Create a new remote candidate or use the one that already exists. */
    remote_cand = findRemoteCandidate(rip, rport);
    if (NULL == remote_cand) {
      remote_cand = new Candidate(rip, rport);
      if (NULL == remote_cand) {
        printf("ice::Stream::createPair() - error: cannot allocate an ice::Candidate. \n");
        return NULL;
      }
    }

    /* Create a new pair of these local and remote candidates. */
    pair = new ice::CandidatePair(local_cand, remote_cand);
    if (NULL == pair) {
      printf("ice::Agent::handleStunMessage() - error: cannot allocate an ice::CandidatePair.\n");
      return NULL;
    }
      
    /* Make sure the stream keeps track of the allocate candidate/pairs. These will be freed by the stream. */
    addRemoteCandidate(remote_cand);
    addCandidatePair(pair);

    return pair;
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

  int Stream::sendRTP(uint8_t* data, uint32_t nbytes) {

    /* validate  */
    if (!data) { return -1; }
    if (!nbytes) { return -2; } 
    if (0 == pairs.size()) {
      printf("ice::Stream::sendRTP() - error: cannot send because we have not pairs yet.\n");
      return -3;
    }
    
    CandidatePair* pair = pairs[0];
    int len = pair->local->srtp_out.protectRTP(data, nbytes);
    if (len < 0) {
      printf("ice::Stream::sendRTP() - verbose: cannot protect the RTP data. Probably the srtp parser is not yet initialized.\n");
      return -4;
    }

    for (size_t i = 0; i < pairs.size(); ++i) {
      pair = pairs[i];
      pair->local->conn.sendTo(pair->remote->ip, pair->remote->port, data, len);
    }

    return 0;
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

    Stream* stream = static_cast<Stream*>(user);
    if (stream->on_data) {
      stream->on_data(stream, rip, rport, lip, lport, data, nbytes, stream->user_data);
    }
  }

} /* namespace ice */

