#include <ice/Stream.h>

namespace ice {

  Stream::Stream() 
    :state(STREAM_STATE_NONE)
  {

  }

  Stream::~Stream() {

    std::vector<Candidate*>::iterator it = candidates.begin();
    while (it != candidates.end()) {
      delete *it;
      it = candidates.erase(it);
    }

  }

  bool Stream::init() {

    for (size_t i = 0; i < candidates.size(); ++i) {
      if (!candidates[i]->init()) {
        return false;
      }
    }

    return true;
  }

  void Stream::update() {
    for (size_t i = 0; i < candidates.size(); ++i) {
      candidates[i]->update();
    }
  }

  void Stream::addCandidate(Candidate* c) {
    candidates.push_back(c);
  }

  void Stream::setCredentials(std::string ufrag, std::string pwd) {
    for (size_t i = 0; i < candidates.size(); ++i) {
      candidates[i]->setCredentials(ufrag, pwd);
    }
  }

} /* namespace ice */
