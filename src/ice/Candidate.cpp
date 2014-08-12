#include <ice/Candidate.h>
#include <stun/Types.h>
#include <stun/Attribute.h>
#include <stun/Writer.h>

namespace ice {

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
  }

  CandidatePair::CandidatePair(Candidate* local, Candidate* remote)
    :local(local)
    ,remote(remote)
  {
  }

  CandidatePair::~CandidatePair() {
    local = NULL;
    remote = NULL;
  }

} /* namespace ice */
