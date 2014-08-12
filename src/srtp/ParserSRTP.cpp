#include <string.h>
#include <stdlib.h>
#include <srtp/ParserSRTP.h>
#include <openssl/tls1.h>  /* for the cipher suites */

namespace srtp {

  bool ParserSRTP::is_lib_init = false;

  ParserSRTP::ParserSRTP()
    :is_init(false)
  {

    /* Initialize the srtp library. */
    if (false == is_lib_init) {
      err_status_t err = srtp_init();
      if (err != err_status_ok) {
        printf("ParserSRTP::ParserSRTP() - error: cannot initialize libsrtp(): %d\n", err);
        exit(1);
      }
      is_lib_init = true;
    }
  }

  ParserSRTP::~ParserSRTP() {

    if (policy.key) {
      delete[] policy.key;
      policy.key = NULL;
    }

    if (is_init) {
      srtp_dealloc(session);
    }

    is_init = false;
  }

  int ParserSRTP::init(const char* cipher, bool inbound, const uint8_t* key, const uint8_t* salt) {
    err_status_t err;

    if (!cipher) { return -1; } 
    if (!key) { return -2; } 
    if (!salt) { return -3; } 
    if (true == is_init) { return -4; } 

    if (0 == strcasecmp(cipher, "SRTP_AES128_CM_SHA1_80")) {
      crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtp);
      crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);
    }
    else if (0 == strcasecmp(cipher, "SRTP_AES128_CM_SHA1_32")) {
      crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy.rtp);
      crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);      /* see http://dxr.mozilla.org/mozilla-central/source/media/webrtc/signaling/src/mediapipeline/SrtpFlow.cpp#228 @todo check if this is correct! */
      printf("srtp::ParserSRTP::init() - warning, we're using crypto_policy_set_aes_cm_128_hmac_sha1_80() .. but didn't test this; \n");
    }
    else {
      printf("srtp::ParserSRTP::init() - error: invalid/unsupported cipher %s\n", cipher);
      return -5;
    }

    /* Allocate space for the key! */
    policy.key = new uint8_t[SRTP_PARSER_MASTER_LEN];      
    if (!policy.key) {
      printf("srtp::ParserSRTP - error: cannot alloc the key for the policy.\n");
      return -6;
    }

    policy.ssrc.type = (true == inbound) ? ssrc_any_inbound : ssrc_any_outbound;    
    policy.window_size = 128;                                                     /* @todo  http://mxr.mozilla.org/mozilla-central/source/media/webrtc/signaling/src/mediapipeline/SrtpFlow.cpp */
    policy.allow_repeat_tx = 0;
    policy.next = NULL;

    /* Copy the key */
    memcpy(policy.key, key, SRTP_PARSER_MASTER_KEY_LEN);
    memcpy(policy.key + SRTP_PARSER_MASTER_KEY_LEN, salt, SRTP_PARSER_MASTER_SALT_LEN);

    err = srtp_create(&session, &policy);
    if (err != err_status_ok) {
      printf("srtp::ParserSRTP - error: cannot create a policy: %d\n", err);
      return -7;
    }
    
    is_init = true;
    
    return 0;
  }

  int ParserSRTP::protectRTP(void* in, uint32_t nbytes) {
    err_status_t err;
    int len = nbytes;

    if (!in) { return -1; }
    if (!nbytes) { return -2; } 

    if (false == is_init) {
      printf("srtp::ParserSRTP::protectRTP() - error: trying to protect data, but we're not initialized.\n");
      return -3;
    }

    err = srtp_protect(session, in, &len);
    if (err != err_status_ok) {
      printf("srtp::ParserSRTP::protectRTP() - error: cannot protect the given srtp packet: %d\n", err);
      return -3;
    }

    return len;
  }

  int ParserSRTP::protectRTCP(void* in, uint32_t nbytes) {
    /* @todo implement */
    return 0;
  }

  int ParserSRTP::unprotectRTP(void* in, uint32_t nbytes) {
    err_status_t err;
    int len = nbytes;

    /* validate. */
    if (!in) { return -1; } 
    if (!nbytes) { return -2; } 

    if (false == is_init) {
      printf("srtp::ParserSRTP::unprotectRTP() - error: trying to unprotect data, but we're not initialized.\n");
      return -3;
    }

    err = srtp_unprotect(session, in, &len);
    if (err != err_status_ok) {
      printf("srtp::ParserSRTP::unprotectRTP() - error: cannot unprotect the given SRTP packet: %d\n", err);
      return -4;
    }
    
    printf("srtp::ParserSRTP::unprotectRTP() - verbose: successfully unprotected a SRTP packet! length: %d\n", len);

    return len;
  }

  int ParserSRTP::unprotectRTCP(void* in, uint32_t nbytes) {
    /* @todo implement */
    return 0;
  }
  
} /* namespace srtp */
