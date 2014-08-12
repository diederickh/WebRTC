#include <ice/Candidate.h>
#include <stun/Types.h>
#include <stun/Attribute.h>
#include <stun/Writer.h>

namespace ice {

  static void ice_candidate_pair_on_dtls_data(uint8_t* data, uint32_t nbytes, void* user);                   /* gets called whenever the dtls connection needs to send some data back to the other party. */  

  Candidate::Candidate(std::string ip, uint16_t port)
    :ip(ip)
    ,port(port)
    ,on_data(NULL)
    ,user(NULL)
  {
  }

  bool Candidate::init(connection_on_data_callback cb, void* user) {

    if (!ip.size()) {
      printf("ice::Candidate - error: candidate::setup(), invalid ip (empty).\n");
      return false;
    }

    if (!port || port < 1024) {
      printf("ice::Candidate - error: candidate::setup(), invalid port; make sure the port is > 1024, now %d\n", port);
      return false;
    }

    if (!conn.bind(ip, port)) {
      return false;
    }

    if (!dtls.init()) {
      return false;
    }

    conn.on_data = cb;
    conn.user = user;

    return true;
  }

  void Candidate::update() {
    conn.update();
  }

  /* ------------------------------------------------------------- */

  CandidatePair::CandidatePair()
    :local(NULL)
    ,remote(NULL)
  {
    dtls.on_data = ice_candidate_pair_on_dtls_data;
    dtls.user = this;
  }

  CandidatePair::CandidatePair(Candidate* local, Candidate* remote)
    :local(local)
    ,remote(remote)
  {
    dtls.on_data = ice_candidate_pair_on_dtls_data;
    dtls.user = this;
  }

  CandidatePair::~CandidatePair() {
    local = NULL;
    remote = NULL;
  }

  /* ------------------------------------------------------------- */

  static void ice_candidate_pair_on_dtls_data(uint8_t* data, uint32_t nbytes, void* user) {
    printf("ice_candidate_pair_on_dtls_data - verbose: receive dtls data that we need to send, %u bytes.\n", nbytes);
    ice::CandidatePair* pair = static_cast<ice::CandidatePair*>(user);
    pair->local->conn.sendTo(pair->remote->ip, pair->remote->port, data, nbytes);
  }

} /* namespace ice */
