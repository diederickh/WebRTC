#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <rtc/DTLS.h>
#include <rtc/Connection.h>
#include <ice/ICE.h>
#include <stun/Reader.h>
#include <ice/Utils.h>
#include <sdp/Types.h>
#include <sdp/SDP.h>
#include <sdp/Reader.h>
#include <sdp/Writer.h>

static void on_udp_data(uint8_t* data, uint32_t nbytes, void* user);  /* gets called when we recieve data on our 'candidate' */
static void on_stun_message(stun::Message* msg, void* user);          /* gets called when we receive a stun message */
static void on_ice_data(uint8_t* data, uint32_t nbytes, void* user);  /* gets called when we need to send ICE related data */

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
  ice::ICE ice;
  rtc::DTLS dtls; 
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

  if (reader.parse(input_sdp, &offer) < 0) {
    printf("Error: cannot parse input SDP.\n");
    exit(1);
  }
  
  /* manipulate the offer so we can use it as an answer. */
  {
    
    if (!dtls.createKeyAndCertificate()) {
      exit(2);
    }

    if (!dtls.createFingerprint(fingerprint)) {
      exit(3);
    }
    
    ice_ufrag = ice::gen_random_string(10);
    ice_pwd = ice::gen_random_string(32);
    
    if (offer.find(sdp::SDP_VIDEO, &video)) {
      printf("- found video. %p\n", video);
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
      printf("- found audio. %p\n", audio);
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
  stun.on_message = on_stun_message;
  stun.user = (void*)&ice;
  stun.password = ice_pwd;
  ice.on_data = on_ice_data;
  ice.user = (void*)&sock;

  /* start receiving data */
  while (true) {
    sock.update();
  }

  return 0;
}

static void on_udp_data(uint8_t* data, uint32_t nbytes, void* user) {
  stun::Reader* stun = static_cast<stun::Reader*>(user);
  stun->process(data, nbytes);
}

static void on_stun_message(stun::Message* msg, void* user) {
  printf("Message.\n");
  msg->computeMessageIntegrity("Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC");

  ice::ICE* ice = static_cast<ice::ICE*>(user);
  ice->handleMessage(msg);
}

static void on_ice_data(uint8_t* data, uint32_t nbytes, void* user) {
  rtc::ConnectionUDP* sock = static_cast<rtc::ConnectionUDP*>(user);
  printf("Must send %u bytes back to ICE sock.\n", nbytes);
  sock->send(data, nbytes);
}
