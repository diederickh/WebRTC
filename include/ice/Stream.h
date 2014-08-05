#ifndef ICE_STREAM_H
#define ICE_STREAM_H

#include <vector>
#include <ice/Candidate.h>

namespace ice {

  enum StreamState {
    STREAM_STATE_NONE = 0x00,
  };

  class Stream {
  public:
    Stream();
    ~Stream();
    bool init();
    void update();
    void addCandidate(Candidate* c);                               /* add a candidate; we take ownership of the candidate and free it in the d'tor. */
    void setCredentials(std::string ufrag, std::string pwd);       /* set the credentials (ice-ufrag, ice-pwd) for all candidates. */

  public:
    StreamState state;
    std::vector<Candidate*> candidates;
  }; 

} /* namespace ice */

#endif
