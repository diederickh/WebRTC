/*
  test_ice
  ---------
  
  This is used during development of the library; it contains ugly snippets of 
  code to try/experiment with the different parts of libwebrtc. This is not 
  meant to be used by end-users of the library. This is also not a unit test.

 */
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <dtls/Context.h>
#include <dtls/Parser.h>
#include <rtc/Connection.h>
#include <stun/Reader.h>
#include <ice/Utils.h>
#include <sdp/Types.h>
#include <sdp/SDP.h>
#include <sdp/Reader.h>
#include <sdp/Writer.h>

#define PASSWORD "Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC"  /* our ice-pwd value */

static void on_udp_data(std::string rip, uint16_t rport, std::string lip, uint16_t lport, uint8_t* data, uint32_t nbytes, void* user);             /* gets called when we recieve data on our 'candidate' */
static void on_dtls_data(uint8_t* data, uint32_t nbytes, void* user);            /* gets called when we need to send DTLS related data */ 

rtc::ConnectionUDP* udp_ptr = NULL;
dtls::Parser* dtls_parser_ptr = NULL;
bool stun_done = false;

int main() {

  printf("\n\ntest_ice\n\n");

  /* read SDP file. */
  std::ifstream ifs("stream.sdp", std::ios::in);
  if(!ifs.is_open()) {
    printf("Error: cannot read stream.sdp.\n");
    exit(1);
  }

  std::string input_sdp( (std::istreambuf_iterator<char>(ifs)) , std::istreambuf_iterator<char>());
  if (input_sdp.size() == 0) {
    printf("Error: input SDP is empty.\n");
    exit(1);
  }

  /* parse the input */
  dtls::Context dtls_ctx; 
  dtls::Parser dtls_parser;
  rtc::ConnectionUDP sock;
  stun::Reader stun;
 
  sdp::SDP offer;
  sdp::Reader reader;
  sdp::Writer writer;
  sdp::Media* audio;
  sdp::Media* video;
  std::string output_sdp;
  std::string ice_ufrag;
  std::string ice_pwd;
  std::string fingerprint;

  udp_ptr = &sock;
  dtls_parser_ptr = &dtls_parser;

  if (reader.parse(input_sdp, &offer) < 0) {
    printf("Error: cannot parse input SDP.\n");
    exit(1);
  }
  
  /* manipulate the offer so we can use it as an answer. */
  {
    
    if (!dtls_ctx.init("./server-cert.pem", "./server-key.pem")) {
      exit(2);
    }

    dtls_parser.ssl = dtls_ctx.createSSL();
    if(!dtls_parser.init()) {
      printf("Cannot init the dtls parser.\n");
      exit(1);
    }
    if (!dtls_ctx.createFingerprint(fingerprint)) {
      printf("Error: cannot create dtls/fingerprint sdp attribute.\n");
      exit(1);
    }
    
    ice_ufrag = ice::gen_random_string(10);
    ice_pwd = ice::gen_random_string(32);
    
    if (offer.find(sdp::SDP_VIDEO, &video)) {
      video->remove(sdp::SDP_ATTR_CANDIDATE);
      video->remove(sdp::SDP_ATTR_ICE_UFRAG);
      video->remove(sdp::SDP_ATTR_ICE_PWD);
      video->remove(sdp::SDP_ATTR_ICE_OPTIONS);  /* chrome will try to use google-ice which is a bit different then normal ice */
      video->remove(sdp::SDP_ATTR_FINGERPRINT);
      video->remove(sdp::SDP_ATTR_SETUP);
      video->add(new sdp::Attribute("ice-ufrag", ice_ufrag, sdp::SDP_ATTR_ICE_UFRAG));
      video->add(new sdp::Attribute("ice-pwd", ice_pwd, sdp::SDP_ATTR_ICE_PWD));
      video->add(new sdp::Attribute("candidate", "4252876256 1 udp 2122260223 192.168.0.194 59976 typ host"));
      video->add(new sdp::AttributeFingerprint("sha-256", fingerprint));
      video->add(new sdp::AttributeSetup(sdp::SDP_PASSIVE));
    }

    if (offer.find(sdp::SDP_AUDIO, &audio)) {
      audio->remove(sdp::SDP_ATTR_CANDIDATE);
      audio->remove(sdp::SDP_ATTR_ICE_UFRAG);
      audio->remove(sdp::SDP_ATTR_ICE_PWD);
      audio->remove(sdp::SDP_ATTR_ICE_OPTIONS);
      audio->remove(sdp::SDP_ATTR_FINGERPRINT);
      audio->remove(sdp::SDP_ATTR_SETUP);
      audio->add(new sdp::Attribute("ice-ufrag", ice_ufrag, sdp::SDP_ATTR_ICE_UFRAG));
      audio->add(new sdp::Attribute("ice-pwd", ice_pwd, sdp::SDP_ATTR_ICE_PWD));
      audio->add(new sdp::Attribute("candidate", "4252876256 1 udp 2122260223 192.168.0.194 59976 typ host"));
      audio->add(new sdp::AttributeFingerprint("sha-256", fingerprint));
      audio->add(new sdp::AttributeSetup(sdp::SDP_PASSIVE));
    }
  }

  /* and reconstruct the answer */
  {
    output_sdp = writer.toString(&offer);
    printf("\n\n%s\n\n", output_sdp.c_str());
  }

  if (!sock.bind("192.168.0.194", 59976)) {
    exit(1);
  }

  sock.on_data = on_udp_data;
  sock.user = (void*)&stun;

  dtls_parser.on_data = on_dtls_data;
  dtls_parser.user = (void*)&dtls_parser;

  /* start receiving data */
  while (true) {
    sock.update();
  }
  return 0;
}

static void on_udp_data(std::string rip, uint16_t rport, 
                        std::string lip, uint16_t lport, 
                        uint8_t* data, uint32_t nbytes, void* user) 
{
  stun::Message msg;
  stun::Reader* stun = static_cast<stun::Reader*>(user);
  int r = stun->process(data, nbytes, &msg);

  if (r == 0) {
    /* valid stun message */
    msg.computeMessageIntegrity(PASSWORD);
  }
  else if (r == 1) {
    /* other data, e.g. DTLS ClientHello or SRTP data */   
    if (dtls_parser_ptr) {
      dtls_parser_ptr->process(data, nbytes);
    }
  }
  else {
    printf("Error: unhandled stun::Reader::process() result.\n");
    exit(1);
  }
}

static void on_dtls_data(uint8_t* data, uint32_t nbytes, void* user) {
  /* handle dtls data, e.g. send back to sock. */
}
