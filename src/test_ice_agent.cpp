#include <stdio.h>
#include <ice/Agent.h>
#include <ice/Candidate.h>
#include <rtp/ReaderVP8.h>
#include <rtp/PacketVP8.h>
#include <video/AggregatorVP8.h>
#include <video/WriterIVF.h>
#include <uv.h>

#define USE_SEND 0         /* Enable sending of VP8 data */

video::AggregatorVP8* aggregator;
ice::Agent* agent;
ice::Stream* video_stream;
video::WriterIVF* ivf;

#if USE_SEND
extern "C" {
#  include <video_generator.h>
}
#  include <video/EncoderSettings.h>
#  include <video/EncoderVP8.h>
#  include <rtp/WriterVP8.h>
#  define WIDTH 320
#  define HEIGHT 240
#  define FRAMERATE 25
#  define RTP_PACKET_SIZE (1000)

   video::EncoderSettings settings;
   video::EncoderVP8 encoder;
   rtp::WriterVP8 rtp_writer;

static void on_vp8_packet(video::EncoderVP8* enc, const vpx_codec_cx_pkt* pkt, int64_t pts);
static void on_rtp_packet(rtp::PacketVP8* pkt, void* user);

struct test_srtp {
  srtp_policy_t policy;
  srtp_t session;
};

test_srtp* validate_srtp = NULL;

#endif

static void on_rtp_data(ice::Stream* stream, ice::CandidatePair* pair, uint8_t* data, uint32_t nbytes, void* user);

int main() {

  printf("\n\ntest_ice_agent\n\n");

  /* create our stream with the candidates */
  aggregator = new video::AggregatorVP8();
  ivf = new video::WriterIVF();
  agent = new ice::Agent();
  video_stream = new ice::Stream();

  video_stream->on_rtp = on_rtp_data;
  video_stream->user_rtp = aggregator;
  video_stream->addLocalCandidate(new ice::Candidate("192.168.0.194", 59976));
  //video_stream->addLocalCandidate(new ice::Candidate("127.0.0.1", 10001));

  /* add the stream to the agent */
  agent->addStream(video_stream);

  /* set the ice-pwd that is used in the MessageIntegrity attribute of the STUN messages */
  // agent->setCredentials("", "Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC");
  // agent->setCredentials("", "75C96DDDFC38D194FEDF75986CF962A2D56F3B65F1F7");
  agent->setCredentials("", "Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC");

  if (ivf->open("test.ivf", 320, 240, 1, 25) < 0) {
    printf("Error: cannot open output ivf file.\n");
    exit(1);
  }

  /* initialize the agent (e.g. sets up the sockets) */
  if (!agent->init()) {
    exit(1);
  }

#if USE_SEND

  settings.width = WIDTH;
  settings.height = HEIGHT;
  settings.fps_num = 1;
  settings.fps_den = FRAMERATE;

  /* initialize the encoder. */
  if (encoder.init(settings) < 0) {
    printf("main - error: cannot initialize the vp8 encoder.\n");
    exit(1);
  }

  encoder.on_packet = on_vp8_packet;
  rtp_writer.on_packet = on_rtp_packet;

  /* initialize the video generator. */
  video_generator gen;
  if (video_generator_init(&gen, WIDTH, HEIGHT, FRAMERATE) < 0) {
    printf("main - error: cannot initialize the video generator.\n");
    exit(1);
  }

  uint64_t pts;
  uint64_t now = uv_hrtime();
  uint64_t time_started = now;
  uint64_t frame_delay = (1.0 / FRAMERATE) * 1000llu * 1000llu * 1000llu;
  uint64_t frame_timeout = now + frame_delay;

#endif
  
  while (true) {
    agent->update();

#if USE_SEND
    now = uv_hrtime();
    if (now > frame_timeout) {                  
      pts = (uv_hrtime() - time_started) / (1000llu * 1000llu);
      frame_timeout = now + frame_delay;
      video_generator_update(&gen);
      encoder.encode(gen.y, gen.strides[0],
                     gen.u, gen.strides[1],
                     gen.v, gen.strides[2], 
                     pts);
    }
#endif
  }
  
  return 0;
}


static void on_rtp_data(ice::Stream* stream, 
                        ice::CandidatePair* pair, 
                        uint8_t* data, uint32_t nbytes, void* user) 
{

#if 0
  /* TEST: send video dat back! */
  for (size_t i = 0; i < video_stream->pairs.size(); ++i) {
    ice::CandidatePair* pair = video_stream->pairs[i];

    if (pair->srtp_reader.is_initialized) {
      int len = nbytes;
      err_status_t err = srtp_protect(pair->srtp_out.session, data, &len);
      if (err != err_status_ok) {
        printf("on_rtp_packet - error: cannot protect the given srtp packet: %d\n", err);
      }
      else {
        pair->local->conn.sendTo(pair->remote->ip, pair->remote->port, data, len);
      }
    }
  }
#endif

  static uint64_t first_ts = 0;
  int r;
  printf("on_rtp_data - vebose: received RTP data, %u bytes.\n", nbytes);
  video::AggregatorVP8* agg = static_cast<video::AggregatorVP8*>(user);
  
  rtp::PacketVP8 pkt;
  if (rtp::rtp_vp8_decode(data, nbytes, &pkt) < 0) {
    printf("on_rtp_data - error: cannot decode vp8 rtp data.\n");
    return;
  }

  if (0 == first_ts) {
    first_ts = pkt.timestamp;
  }

#if 0
  if (agg->addPacket(&pkt) == 1) {
    r = ivf->write(agg->buffer, agg->nbytes, ivf->nframes);
    if (ivf->nframes >= 2250) {
      ivf->close();
      printf("ready storing frames.\n");
      exit(1);
    }
    if (r < 0) {
      printf("on_rtp_data - error: cannot write frame: %d\n", r);
      exit(1);
    }
  }
#endif
  
}

#if USE_SEND
static void on_vp8_packet(video::EncoderVP8* enc, const vpx_codec_cx_pkt* pkt, int64_t pts) {
  printf("on_vp8_packet - verbose: got vp8 packet: %lld\n", pts);
  rtp_writer.packetize(pkt);
}

static void on_rtp_packet(rtp::PacketVP8* pkt, void* user) {

  printf("\n\n===========================================\n");
  rtp::PacketVP8 test_pkt;
  if (rtp::rtp_vp8_decode(pkt->payload, pkt->nbytes, &test_pkt) < 0) {
    printf("on_rtp_packet: error: cannot decode incoming VP8 RTP packet.\n");
    exit(1);
  }
  printf("on_rtp_packet: verbose: decoded seqnum: %u\n", test_pkt.sequence_number);


  printf("on_rtp_packet - verbose: got RTP packet, and we have %lu pairs.\n", video_stream->pairs.size());
  int ok = 0;
  int len = pkt->nbytes;
  ice::CandidatePair* pair = NULL;
  for (size_t i = 0; i < video_stream->pairs.size(); ++i) {
    pair = video_stream->pairs[i];
    printf("on_rtp_packet - verbose: pair, local ip: %s:%d, remote ip: %s:%d\n",
           pair->local->ip.c_str(),
           pair->local->port,
           pair->remote->ip.c_str(),
           pair->remote->port
           );

    if (false == pair->srtp_out.is_initialized) {
      printf(">>>>>>>>> NOT INITIALIZED <<<<<<<<<<<<<\n");
    }
    else {
      printf(">>>>>>>>>>>>>>>> INITIALIZED <<<<<<<<<<<< \n");
    }
    if (pair->srtp_reader.is_initialized) {
      len = pkt->nbytes;
      err_status_t err = srtp_protect(pair->srtp_out.session, pkt->payload, &len);
      if (err != err_status_ok) {
        ok = 0;
        printf("on_rtp_packet - error: cannot protect the given srtp packet: %d\n", err);
      }

      ok = 1;
      printf(">>> SRTP_PROTECT RESULT: %d\n", err);
      break;
    }
  }

  if (0 == video_stream->pairs.size()) {
    return;
  }
  //ice::CandidatePair* last_pair = video_stream->pairs[video_stream->pairs.size() -1 ];
  ice::CandidatePair* last_pair = video_stream->pairs[0];
  for (size_t i = 0; i < video_stream->pairs.size(); ++i) {
    ice::CandidatePair* p = video_stream->pairs[i];
    printf(">> %s:%u <> %s:%u\n", p->local->ip.c_str(), p->local->port, p->remote->ip.c_str(), p->remote->port);
  }

#if 1
  if (pair && NULL == validate_srtp && NULL != pair->dtls.remote_key)  {

    validate_srtp = new test_srtp;
    crypto_policy_set_aes_cm_128_hmac_sha1_80(&validate_srtp->policy.rtp);   /* @todo see SSL_get_selected_srtp_profile() to extract the name */
    crypto_policy_set_aes_cm_128_hmac_sha1_80(&validate_srtp->policy.rtcp);  /* @todo see SSL_get_selected_srtp_profile() to extract the name */

    validate_srtp->policy.key = new uint8_t[SRTP_READER_MASTER_LEN];         /* @todo - make sure to cleanup somewhere */
    validate_srtp->policy.ssrc.type = ssrc_any_inbound;    
    validate_srtp->policy.window_size = 128;                                 /* @todo  http://mxr.mozilla.org/mozilla-central/source/media/webrtc/signaling/src/mediapipeline/SrtpFlow.cpp */
    validate_srtp->policy.allow_repeat_tx = 0;
    validate_srtp->policy.next = NULL;
    memcpy(validate_srtp->policy.key, pair->dtls.local_key, SRTP_READER_MASTER_KEY_LEN);
    memcpy(validate_srtp->policy.key + SRTP_READER_MASTER_KEY_LEN, pair->dtls.local_salt, SRTP_READER_MASTER_SALT_LEN);

    err_status_t err = srtp_create(&validate_srtp->session, &validate_srtp->policy);
    if (err != err_status_ok) {
      printf("on_rtp_packet: error - cannot init validate srtp.\n");
      exit(1);
    }
  }
#endif   

  if (NULL != validate_srtp && 1 == ok && pair) {
#if 0
    for (size_t i = 0; i < video_stream->remote_candidates.size(); ++i) {
      ice::Candidate* p = video_stream->remote_candidates[i];
      pair->local->conn.sendTo(p->ip, p->port, pkt->payload, len);
    }
#else
    pair->local->conn.sendTo(last_pair->remote->ip, last_pair->remote->port, pkt->payload, len);
#endif
    
    /*
    for (size_t i = 1; i < video_stream->remote_candidates.size(); ++i) {
      ice::Candidate* cand = video_stream->remote_candidates[i];
      printf("on_rtp_packet: remote %s:%d\n", cand->ip.c_str(), cand->port);
      printf("on_rtp_packet: protected! len: %d, nbytes: %d\n", len, pkt->nbytes);
      pair->local->conn.sendTo(cand->ip, cand->port, pkt->payload, len);
      err_status_t err = srtp_unprotect(validate_srtp->session, pkt->payload, &len);
      printf("on_rtp_packet: UNPROTECTED! LEN: %d, ERR: %d\n", len, err);
    }
    */
  }

  printf("===========================================\n\n");
}
#endif
