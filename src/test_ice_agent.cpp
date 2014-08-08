#include <stdio.h>
#include <ice/Agent.h>
#include <ice/Candidate.h>
#include <rtp/ReaderVP8.h>
#include <rtp/PacketVP8.h>
#include <video/AggregatorVP8.h>
#include <video/WriterIVF.h>

video::AggregatorVP8* aggregator;
ice::Agent* agent;
ice::Stream* video_stream;
video::WriterIVF* ivf;

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
  video_stream->addLocalCandidate(new ice::Candidate("192.168.0.193", 59976));
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
  
  while (true) {
    agent->update();
  }
  
  return 0;
}


static void on_rtp_data(ice::Stream* stream, 
                        ice::CandidatePair* pair, 
                        uint8_t* data, uint32_t nbytes, void* user) 
{
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

  if (agg->addPacket(&pkt) == 1) {
    r = ivf->write(agg->buffer, agg->nbytes, ivf->nframes);
    if (ivf->nframes >= 1250) {
      ivf->close();
      printf("ready storing frames.\n");
      exit(1);
    }
    if (r < 0) {
      printf("on_rtp_data - error: cannot write frame: %d\n", r);
      exit(1);
    }
  }
  
}
