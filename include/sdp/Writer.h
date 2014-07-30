#ifndef SDP_WRITER_H
#define SDP_WRITER_H

#include <string>
#include <vector>
#include <sstream>
#include <sdp/SDP.h>
#include <sdp/Types.h>
#include <sdp/Utils.h>

namespace sdp {

  class Writer {
  public:
    std::string toString(SDP* sdp);
    std::string toString(Node* sdp);
    std::string toString(Version* v);
    std::string toString(Origin* o);
    std::string toString(SessionName* s);
    std::string toString(Timing* t);
    std::string toString(Media* m);
    std::string toString(Attribute* a);
    std::string toString(AttributeCandidate* c);
    std::string toString(AttributeFingerprint* f);
    std::string toString(AttributeSetup* f);
  };  

} /* namesapce sdp */
#endif
