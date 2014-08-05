#include <stdio.h>
#include <ice/Agent.h>
#include <ice/Candidate.h>

ice::Agent* agent;
ice::Stream* video_stream;

int main() {

  printf("\n\ntest_ice_agent\n\n");

  /* create our stream with the candidates */
  agent = new ice::Agent();
  video_stream = new ice::Stream();
  video_stream->addCandidate(new ice::Candidate("192.168.0.194", 59976));

  /* add the stream to the agent */
  agent->addStream(video_stream);

  /* set the ice-pwd that is used in the MessageIntegrity attribute of the STUN messages */
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
