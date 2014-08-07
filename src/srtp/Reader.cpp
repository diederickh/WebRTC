#include <srtp/Reader.h>
#include <stdio.h>
#include <string.h>


namespace srtp {

  Reader::Reader() 
    :is_initialized(false)
  {

    /* init the srtp library */
    static bool srtplib_is_init = false;
    if (!srtplib_is_init) {
      err_status_t err;
      err = srtp_init();
      if (err != err_status_ok) {
        printf("srtp::Reader - error: cannot initialize the srtp session: %d\n", err);
        exit(1);
      }
      srtplib_is_init = true;
    }
  }

  Reader::~Reader() {

    if (policy.key) {
      delete[] policy.key;
      policy.key = NULL;
    }

    if (is_initialized) {
      srtp_dealloc(session);
    }

    is_initialized = false;
  }

  bool Reader::init(uint8_t* key, uint8_t* salt) {

    err_status_t err;

    if (is_initialized) {
      printf("srtp::Reader - error: trying to initialize srtp::Reader, but already initialized?\n");
      return false;
    }

    if (!key || !salt) {
      printf("srtp::Reader - error: invalid params, key: %p, salt: %p.\n", key, salt);
      return false;
    }
    
    crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtp);   /* @todo see SSL_get_selected_srtp_profile() to extract the name */
    crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);  /* @todo see SSL_get_selected_srtp_profile() to extract the name */

    policy.key = new uint8_t[SRTP_READER_MASTER_LEN];         /* from janus-gateway, why the + 8 ?, @todo - make sure to cleanup somewhere */
    policy.ssrc.type = ssrc_any_inbound;    
    policy.window_size = 128;                                 /* @todo  http://mxr.mozilla.org/mozilla-central/source/media/webrtc/signaling/src/mediapipeline/SrtpFlow.cpp */
    policy.allow_repeat_tx = 0;
    policy.next = NULL;

    if (!policy.key) {
      printf("srtp::Reader - error: cannot alloc the key for the policy.\n");
      return false;
    }

    /* copy the key */
    memcpy(policy.key, key, SRTP_READER_MASTER_KEY_LEN);
    memcpy(policy.key + SRTP_READER_MASTER_KEY_LEN, salt, SRTP_READER_MASTER_SALT_LEN);
    
    err = srtp_create(&session, &policy);
    if (err != err_status_ok) {
      printf("srtp::Reader - error: cannot create a policy: %d\n", err);
      return false;
    }
    
    is_initialized = true;
    return true;
  }

  bool Reader::process(uint8_t* data, uint32_t nbytes) {

    err_status_t err;
    int len = nbytes;

    if (!is_initialized) {
      printf("srtp::Reader - error: trying to unprotect data, but we're not initialized.\n");
      exit(1);
    }

    err = srtp_unprotect(session, data, &len);
    if (err != err_status_ok) {
      printf("srtp::Reader - error: cannot unprotect the given SRTP packet: %d\n", err);
      return false;
    }
    
    printf("srtp::Reader - verbose: successfully unprotected a SRTP packet! length: %d\n", len);

    return true;
  }

} /* namespace srtp */
