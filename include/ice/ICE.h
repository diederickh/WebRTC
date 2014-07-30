#ifndef ICE_H
#define ICE_H

#include <string>
#include <vector>

namespace ice {

  class ICE {
  public:
    ICE();
    //    void addCandidate(std::string c); /* add a a=candidate: .. */
    std::string getSDP();             /* returns the SDP for the ICE elements */
    void process(uint8_t* data, uint32_t nbytes);

  public:
    //std::vector<std::string> candidates;
  };

} /* namespace ice */
#endif
