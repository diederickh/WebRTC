#include <ice/Candidate.h>
#include <stun/Types.h>
#include <stun/Attribute.h>
#include <stun/Writer.h>

namespace ice {

  /* ------------------------------------------------------------- */
  static void ice_candidate_on_sock_data(uint8_t* data, uint32_t nbytes, void* user);                   /* gets called whenever we receive some data from the socket */
  static void ice_candidate_on_stun_message(stun::Message* msg, void* user);                            /* gets called whenever the stun::Reader parsed the incoming data and found a valid stun-message. */
  static void ice_candidate_on_stun_pass_through(uint8_t* data, uint32_t nbytes, void* user);           /* gets called whenever the stun::Reader tried to parse incoming data, but marked the data as a non-stun message (e.g. DTLS handshake). */  
  static void ice_candidate_on_dtls_data(uint8_t* data, uint32_t nbytes, void* user);                   /* gets called whenever the dtls connection needs to send some data back to the other party. */
  /* ------------------------------------------------------------- */

  Candidate::Candidate(std::string ip, uint16_t port)
    :state(CANDIDATE_STATE_NONE)
    ,ip(ip)
    ,port(port)
  {
  }

  bool Candidate::init() {

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

    stun.on_message = ice_candidate_on_stun_message;
    stun.on_pass_through = ice_candidate_on_stun_pass_through;
    stun.user = (void*) this;
    conn.on_data = ice_candidate_on_sock_data;
    conn.user = (void*) this;
    dtls.on_data = ice_candidate_on_dtls_data;
    dtls.user = (void*) this;

    state = CANDIDATE_STATE_WAITING;

    return true;
  }

  void Candidate::setCredentials(std::string ufrag, std::string pwd) {

    if (0 == ufrag.size()) {
      printf("ice::Candidate - warning: the ice_ufrag given to setCredentials is empty.\n");
    }

    if (0 == pwd.size()) {
      printf("ice::Candidate - warning: the ice_pwd given to setCredentials is empty.\n");
    }

    ice_ufrag = ufrag;
    ice_pwd = pwd;
  }

  void Candidate::update() {
    conn.update();
  }

  /* see: http://tools.ietf.org/html/rfc5245#section-5.7.4 */
  static void ice_candidate_on_stun_message(stun::Message* msg, void* user) {

    printf("ice::Candidate - verbose: received stun message in Candidate, type: %s\n", stun::message_type_to_string(msg->type).c_str());

    Candidate* cand = static_cast<Candidate*>(user);

    switch (cand->state) {

      case CANDIDATE_STATE_WAITING: {

        if (msg->type != stun::STUN_BINDING_REQUEST) {
          printf("ice::Candidate - error - we only implement ice-controlled mode for now so we must receive a STUN_BINDING_REQUEST first.\n");
          exit(0);
        }
        
        if (!msg->hasAttribute(stun::STUN_ATTR_USE_CANDIDATE)) {
          printf("ice::Candidate - error - only implementing controlled server side of ice; not STUN_ATTR_USE_CANDIDATE attribute found.\n");
          exit(0);
        }

        /* Construct our STUN Binding-Success-Response */
        stun::Message response(stun::STUN_BINDING_RESPONSE);
        response.copyTransactionID(msg);
        response.addAttribute(new stun::XorMappedAddress(cand->ip, cand->port));
        response.addAttribute(new stun::MessageIntegrity());
        response.addAttribute(new stun::Fingerprint());

        stun::Writer writer;
        writer.writeMessage(&response, cand->ice_pwd);
        cand->conn.send(&writer.buffer[0], writer.buffer.size());
        
        printf("ice::Canidate - warning: received STUN_BINDING_REQUEST and replied with STUN_BINDING_RESPONSE, now switching state to CANDIDATE_STATE_SUCCEEDED.\n");
        cand->state = CANDIDATE_STATE_SUCCEEDED;
        break;
      }

      default: {
        printf("ice::Candidate - verbose: unhandled candidate state.\n");
        break;
      }
    }
  }

  static void ice_candidate_on_sock_data(uint8_t* data, uint32_t nbytes, void* user) {
    printf("ice::Candidate - verbose: received data from socket in Candidate. %u bytes.\n", nbytes);
  
    ice::Candidate* cand = static_cast<ice::Candidate*>(user);
    cand->stun.process(data, nbytes);
  }

  static void ice_candidate_on_stun_pass_through(uint8_t* data, uint32_t nbytes, void* user) {
    printf("ice::Candidate - verbose: recieved data which is not a stun message. %u bytes\n", nbytes);
    ice::Candidate* cand = static_cast<ice::Candidate*>(user);
    cand->dtls.process(data, nbytes);
  }

  static void ice_candidate_on_dtls_data(uint8_t* data, uint32_t nbytes, void* user) {
    printf("ice::Candidate - verbose: receive dtls data that we need to send, %u bytes.\n", nbytes);
    ice::Candidate* cand = static_cast<ice::Candidate*>(user);
    cand->conn.send(data, nbytes);
  }

} /* namespace ice */
