#ifndef ICE_H
#define ICE_H

#include <string>
#include <vector>
#include <stun/Message.h>

namespace ice {

  class ICE {
  public:
    ICE();
    //    void addCandidate(std::string c); /* add a a=candidate: .. */
    std::string getSDP();             /* returns the SDP for the ICE elements */
    void handleMessage(stun::Message* msg);

  private:
    void handleBindingRequest(stun::Message* msg);

  public:
    //std::vector<std::string> candidates;
  };

} /* namespace ice */
#endif
