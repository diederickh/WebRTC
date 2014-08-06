#include <ice/Candidate.h>
#include <stun/Types.h>
#include <stun/Attribute.h>
#include <stun/Writer.h>

namespace ice {

  /* ------------------------------------------------------------- */
  /* @ TODO - THESE WILL PROBABLY BE MOVED TO THE STREAM CLASS OR HANDLED IN CANDIDATE-PAIR (!?) */
  static void ice_candidate_on_sock_data(std::string ip, uint16_t port, uint8_t* data, uint32_t nbytes, void* user);                   /* gets called whenever we receive some data from the socket */
  //  static void ice_candidate_on_stun_message(stun::Message* msg, void* user);                            /* gets called whenever the stun::Reader parsed the incoming data and found a valid stun-message. */
  //  static void ice_candidate_on_stun_pass_through(uint8_t* data, uint32_t nbytes, void* user);           /* gets called whenever the stun::Reader tried to parse incoming data, but marked the data as a non-stun message (e.g. DTLS handshake). */  
  static void ice_candidate_on_dtls_data(uint8_t* data, uint32_t nbytes, void* user);                   /* gets called whenever the dtls connection needs to send some data back to the other party. */
  /* ------------------------------------------------------------- */

  static void ice_candidate_pair_on_stun_message(stun::Message* msg, void* user);                            /* gets called whenever the stun::Reader parsed the incoming data and found a valid stun-message. */
  static void ice_candidate_pair_on_stun_pass_through(uint8_t* data, uint32_t nbytes, void* user);           /* gets called whenever the stun::Reader tried to parse incoming data, but marked the data as a non-stun message (e.g. DTLS handshake). */  
  static void ice_candidate_pair_on_dtls_data(uint8_t* data, uint32_t nbytes, void* user);                   /* gets called whenever the dtls connection needs to send some data back to the other party. */  

  Candidate::Candidate(std::string ip, uint16_t port)
    :state(CANDIDATE_STATE_NONE)
    ,ip(ip)
    ,port(port)
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

    // stun.on_message = ice_candidate_on_stun_message;
    // stun.on_pass_through = ice_candidate_on_stun_pass_through;
    // stun.user = (void*) this;
    //    conn.on_data = ice_candidate_on_sock_data;
    conn.on_data = cb;
    conn.user = user;
    //  conn.user = (void*) this;
    //    dtls.on_data = ice_candidate_on_dtls_data;
    //dtls.user = (void*) this;

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


  static void ice_candidate_on_sock_data(std::string ip, uint16_t port, uint8_t* data, uint32_t nbytes, void* user) {
    printf("ice::Candidate - verbose: received data from socket in Candidate. %u bytes.\n", nbytes);
  
    ice::Candidate* cand = static_cast<ice::Candidate*>(user);
    cand->stun.process(data, nbytes);
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
    stun.on_message = ice_candidate_pair_on_stun_message;
    stun.on_pass_through = ice_candidate_pair_on_stun_pass_through;
    stun.user = this;

    /* ----------- testing ---------------------- */
    if (!dtls_ctx.init("./server-cert.pem", "./server-key.pem")) {
      printf("ice::CandidatePair - error: cannot initialize the dtls context.\n");
      exit(1);
    }

    dtls.ssl = dtls_ctx.createSSL();
    if (!dtls.ssl) {
      printf("ice::CandidatePair - error: cannot create SSL.\n");
      exit(1);
    }
    
    if (!dtls.init()) {
      printf("ice::CandidatePair - error: cannot init dtls.\n");
      exit(1);
    }

    dtls.on_data = ice_candidate_pair_on_dtls_data;
    dtls.user = this;

    /* ----------- testing ---------------------- */
    printf("Created DTLS pairs + context.\n");
  }

  CandidatePair::~CandidatePair() {
    local = NULL;
    remote = NULL;
  }

  void CandidatePair::process(uint8_t* data, uint32_t nbytes) {
    stun.process(data, nbytes);
  }

  /* ------------------------------------------------------------- */

  /* see: http://tools.ietf.org/html/rfc5245#section-5.7.4 */
  static void ice_candidate_pair_on_stun_message(stun::Message* msg, void* user) {

    printf("ice::Candidate - verbose: received stun message in Candidate, type: %s\n", stun::message_type_to_string(msg->type).c_str());

    CandidatePair* pair = static_cast<CandidatePair*>(user);

    if (msg->type != stun::STUN_BINDING_REQUEST) {
      printf("ice_candidate_pair_on_stun_message - error - we only implement ice-controlled mode for now so we must receive a STUN_BINDING_REQUEST first.\n");
      exit(0);
    }

#if 0        
    if (!msg->hasAttribute(stun::STUN_ATTR_USE_CANDIDATE)) {
      printf("ice_candidate_pair_on_stun_message - error - only implementing controlled server side of ice; no STUN_ATTR_USE_CANDIDATE attribute found.\n");
      return;
    }
#endif

    /* Construct our STUN Binding-Success-Response */
    stun::Message response(stun::STUN_BINDING_RESPONSE);
    response.copyTransactionID(msg);
    response.addAttribute(new stun::XorMappedAddress(pair->remote->ip, pair->remote->port));
    response.addAttribute(new stun::MessageIntegrity());
    response.addAttribute(new stun::Fingerprint());

    stun::Writer writer;
    writer.writeMessage(&response, pair->local->ice_pwd);
    pair->local->conn.sendTo(pair->remote->ip, pair->remote->port,
                             &writer.buffer[0], writer.buffer.size());
        
    printf("ice_candidate_pair_on_stun_message - warning: received STUN_BINDING_REQUEST and replied with STUN_BINDING_RESPONSE, now switching state to CANDIDATE_STATE_SUCCEEDED.\n");
    
  }

  static void ice_candidate_pair_on_stun_pass_through(uint8_t* data, uint32_t nbytes, void* user) {
    printf("ice_candidate_pair_on_stun_pass_throug - verbose: recieved data which is not a stun message. %u bytes\n", nbytes);
    CandidatePair* pair = static_cast<CandidatePair*>(user);
    pair->dtls.process(data, nbytes);
  }

  static void ice_candidate_pair_on_dtls_data(uint8_t* data, uint32_t nbytes, void* user) {
    printf("ice::Candidate - verbose: receive dtls data that we need to send, %u bytes.\n", nbytes);
    ice::CandidatePair* pair = static_cast<ice::CandidatePair*>(user);
    pair->local->conn.sendTo(pair->remote->ip, pair->remote->port, data, nbytes);
  }

} /* namespace ice */
