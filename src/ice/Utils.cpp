#include <ice/Utils.h>

namespace ice {

  std::string gen_random_string(const int len) {
    std::string s;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
      s.push_back(alphanum[rand() % (sizeof(alphanum) - 1)]);
    }

    return s;
  }

} /* namespace ice */
