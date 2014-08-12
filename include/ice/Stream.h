#ifndef ICE_STREAM_H
#define ICE_STREAM_H

#include <vector>
#include <ice/Candidate.h>
#include <dtls/Parser.h>

namespace ice {

  class Stream;

  /* gets called whenever a cadidate pair receives data from the socket. */
  //typedef void(*stream_data_callback)(Stream* stream, CandidatePair* pair, uint8_t* data, uint32_t nbytes, void* user);

  typedef void(*stream_data_callback)(Stream* stream, 
                                      std::string rip, uint16_t rport, 
                                      std::string lip, uint16_t lport,
                                      uint8_t* data, uint32_t nbytes, void* user);
                                      
  typedef void(*stream_media_callback)(Stream* stream, CandidatePair* pair, 
                                       uint8_t* data, uint32_t nbytes, void* user);
                                      

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
    CandidatePair* createPair(std::string rip, uint16_t rport, std::string lip, uint16_t lport);/* creates a new candidate pair for the given IPs, ofc. when the local stream exists */ 
    CandidatePair* findPair(std::string rip, uint16_t rport, std::string lip, uint16_t lport);  /* used internally to find a pair on which data flows */
    Candidate* findLocalCandidate(std::string ip, uint16_t port);                               /* find a local candidate for the given local ip and port. */
    Candidate* findRemoteCandidate(std::string ip, uint16_t port);                              /* find a remote candidate for the given remote ip and port. */
    int sendRTP(uint8_t* data, uint32_t nbytes);                                                /* send unprotected RTP data; we will make sure it's protected. */

  public:
    std::vector<Candidate*> local_candidates;                                                   /* our local candidates */
    std::vector<Candidate*> remote_candidates;                                                  /* our remote candidates */
    std::vector<CandidatePair*> pairs;                                                          /* the candidate pairs */
    stream_data_callback on_data;                                                               /* the stream data callback; is called whenever one of the transports receives data; the Agent handles incoming data. */
    stream_media_callback on_rtp;                                                               /* is called whenever there is decoded rtp data; it's up to the user to call this at the right time, e.g. see Agent.cpp */
    void* user_data;                                                                            /* user data that is passed to the on_data handler. */
    void* user_rtp;                                                                             /* user data that is passed to the on_rtp handler. */
    std::string ice_ufrag;                                                                      /* the ice_ufrag from the sdp */
    std::string ice_pwd;                                                                        /* the ice-pwd value from the sdp, used when adding the message-integrity element to the responses. */ 
  }; 

} /* namespace ice */

#endif
