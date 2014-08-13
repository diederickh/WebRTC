#include <uv.h>
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

  std::vector<std::string> get_interface_addresses() {
    std::vector<std::string> result;
    char buf[512];
    int count, i;
    uv_interface_address_t* info;
    uv_interface_addresses(&info, &count);

    for (i = 0; i < count; ++i) {

      /* skip internal ones, e.g. loopback . */
      uv_interface_address_t iface = info[i];
      if (0 != iface.is_internal) {
        continue;
      }

      /* IP4 type addresses. */
      if (iface.address.address4.sin_family == AF_INET) {
        uv_ip4_name(&iface.address.address4, buf, sizeof(buf));
        result.push_back(buf);
      }
    }

    uv_free_interface_addresses(info, count);

    return result;
  }

} /* namespace ice */
