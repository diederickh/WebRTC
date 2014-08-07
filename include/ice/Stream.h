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
    bool init();                                                                                /* initialize, must be called once after all local candidates have been added */
    void update();                                                                              /* must be called often, which flush any pending buffers */
    void addLocalCandidate(Candidate* c);                                                       /* add a candidate; we take ownership of the candidate and free it in the d'tor. */
    void addRemoteCandidate(Candidate* c);                                                      /* add a remote candidate; is done whenever we recieve data from a ip:port for which no CandidatePair exists. */ 
    void addCandidatePair(CandidatePair* p);                                                    /* add a candidate pair; local -> remote data flow */
    void setCredentials(std::string ufrag, std::string pwd);                                    /* set the credentials (ice-ufrag, ice-pwd) for all candidates. */
    CandidatePair* findPair(std::string lip, uint16_t lport, std::string rip, uint16_t rport);  /* used internally to find a pair on which data flows */
    Candidate* findLocalCandidate(std::string ip, uint16_t port);                               /* find a local candidate for the given local ip and port. */
    Candidate* findRemoteCandidate(std::string ip, uint16_t port);                              /* find a remote candidate for the given remote ip and port. */

  public:
    StreamState state;
    std::vector<Candidate*> local_candidates;                                                   /* our local candidates */
    std::vector<Candidate*> remote_candidates;                                                  /* our remote candidates */
    std::vector<CandidatePair*> pairs;                                                          /* the candidate pairs */
    stream_data_callback on_data;                                                               /* the stream data callback; is called whenever one of the transports receives data; the Agent handles incoming data. */
    void* user;                                                                                 /* gets passed into the on_data callback. */
  }; 

} /* namespace ice */

#endif
