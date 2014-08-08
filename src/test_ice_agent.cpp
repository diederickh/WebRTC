#include <stdio.h>
#include <ice/Agent.h>
#include <ice/Candidate.h>
#include <rtp/ReaderVP8.h>

ice::Agent* agent;
ice::Stream* video_stream;

static void on_rtp_data(ice::Stream* stream, ice::CandidatePair* pair, uint8_t* data, uint32_t nbytes, void* user);

int main() {

  printf("\n\ntest_ice_agent\n\n");

  /* create our stream with the candidates */
  agent = new ice::Agent();
  video_stream = new ice::Stream();
  video_stream->on_rtp = on_rtp_data;
  video_stream->user_rtp = agent;
  video_stream->addLocalCandidate(new ice::Candidate("192.168.0.193", 59976));
  //video_stream->addLocalCandidate(new ice::Candidate("127.0.0.1", 10001));

  /* add the stream to the agent */
  agent->addStream(video_stream);

  /* set the ice-pwd that is used in the MessageIntegrity attribute of the STUN messages */
  // agent->setCredentials("", "Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC");
  // agent->setCredentials("", "75C96DDDFC38D194FEDF75986CF962A2D56F3B65F1F7");
  agent->setCredentials("", "Q9wQj99nsQzldVI5ZuGXbEWRK5RhRXdC");


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
  printf("on_rtp_data - vebose: received RTP data, %u bytes.\n", nbytes);

  rtp::ReaderVP8 vp8_rtp;
  if (vp8_rtp.process(data, nbytes) < 0) {
    printf("on_rtp_data - error: cannot decode vp8 rtp data.\n");
  }
}
