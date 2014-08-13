#ifndef ICE_UTILS_H
#define ICE_UTILS_H

#include <random>
#include <string>
#include <vector>

namespace ice {

  std::string gen_random_string(const int len);        /* generates a random alpha-num string with the given len. */
  std::vector<std::string> get_interface_addresses();  /* retrieve interface addresses, can be used to create a SDP.*/
 
} /* namespace ice */
#endif
