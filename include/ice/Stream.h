#ifndef ICE_STREAM_H
#define ICE_STREAM_H

#include <vector>
#include <ice/Candidate.h>
#include <dtls/Parser.h>

namespace ice {

  class Stream;
  typedef void(*stream_data_callback)(Stream* stream, CandidatePair* pair, uint8_t* data, uint32_t nbytes);

  enum StreamState {
    STREAM_STATE_NONE = 0x00,
  };

  class Stream {
  public:
    Stream();
    ~Stream();
    bool init();
    void update();
    void addLocalCandidate(Candidate* c);                          /* add a candidate; we take ownership of the candidate and free it in the d'tor. */
    void addRemoteCandidate(Candidate* c);                         /* add a remote candidate; is done whenever we recieve data from a ip:port for which no CandidatePair exists. */ 
    void addCandidatePair(CandidatePair* p);
    void setCredentials(std::string ufrag, std::string pwd);       /* set the credentials (ice-ufrag, ice-pwd) for all candidates. */
    CandidatePair* findPair(std::string lip, uint16_t lport, std::string rip, uint16_t rport);
    Candidate* findLocalCandidate(std::string ip, uint16_t port);
    Candidate* findRemoteCandidate(std::string ip, uint16_t port);

  public:
    StreamState state;
    std::vector<Candidate*> local_candidates;                      /* our candidates */
    std::vector<Candidate*> remote_candidates;
    std::vector<CandidatePair*> pairs;                             /* the candidate pairs */
    stream_data_callback on_data;
    void* user;
  }; 

} /* namespace ice */

#endif
