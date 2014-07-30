#ifndef SDP_UTILS_H
#define SDP_UTILS_H

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <sstream>
#include <vector>
#include <sdp/Types.h>

namespace sdp {

  bool string_to_net_type(std::string& input, NetType& result);         /* convert a string to a NetType */
  bool string_to_addr_type(std::string& input, AddrType& result);       /* convert a string to an AddrType */
  bool string_to_media_type(std::string& input, MediaType& result);     /* convert a string to a MediaType */
  bool string_to_media_proto(std::string& input, MediaProto& result);   /* convert a string to a MediaType */
  bool string_to_cand_type(std::string& input, CandType& result);       /* convert a string to a CandType */ 
  bool string_to_setup_type(std::string& input, SetupType& result);     /* convert a string to a SetupType for the a=setup: attribute */
  std::string net_type_to_string(NetType type);
  std::string addr_type_to_string(AddrType type);
  std::string media_type_to_string(MediaType type);
  std::string media_proto_to_string(MediaProto proto);
  std::string cand_type_to_string(CandType type);
  std::string setup_type_to_string(SetupType type);


  /* trim from right */
  inline static std::string rtrim(const std::string &source , const std::string& t = " " ) {
    std::string str = source;
    return str.erase (str.find_last_not_of(t) + 1);
  }

  /* trim from left */
  inline static std::string ltrim(const std::string& source, const std::string& t = " ") {
    std::string str = source;
    return str.erase (0 , source.find_first_not_of(t));
  }

  /* trim from both left and right */
  inline static std::string trim ( const std::string& source, const std::string& t = " ") {
    std::string str = source;
    return ltrim(rtrim (str, t) , t);
  }

  /* tokenizes the input on the given character. */
  inline int tokenize(std::string input, char delim, std::vector<std::string>& output) {

    std::stringstream ss(input);
    std::string line;

    while (std::getline(ss, line, delim)) {
      output.push_back(line);
    }

    if (!output.size()) {
      return -1;
    }

    return 0;

  };

  /* check if the given value is numeric */
  inline bool is_numeric(std::string s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) {
      ++it;
    }
    return !s.empty() && it == s.end();
  }

  /* converts the given string to another type */
  template<class T> T convert(std::string value) {
    T result;
    std::stringstream ss(value);
    ss >> result;
    return result;
  }
};

#endif
