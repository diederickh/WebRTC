#include <sdp/Reader.h>
#include <sdp/Utils.h>

namespace sdp { 

  /* ----------------------------------------------------------- */

  Token::Token() {
  }

  Token::Token(std::string value)
    :value(value)
  {
  }

  size_t Token::size() {
    return value.size();
  }

  bool Token::isNumeric() {
    return sdp::is_numeric(value);
  }

  int Token::toInt() {
    return sdp::convert<int>(value);
  }

  uint64_t Token::toU64() {
    return sdp::convert<uint64_t>(value);
  }

  std::string Token::toString() {
    return value;
  }

  NetType Token::toNetType() {
    NetType result = SDP_NETTYPE_NONE;
    sdp::string_to_net_type(value, result);
    return result;
  }

  AddrType Token::toAddrType() {
    AddrType result;
    sdp::string_to_addr_type(value, result);
    return result;
  }

  MediaType Token::toMediaType() {
    MediaType result;
    sdp::string_to_media_type(value, result);
    return result;
  }

  MediaProto Token::toMediaProto() {
    MediaProto result;
    sdp::string_to_media_proto(value, result);
    return result;
  }

  CandType Token::toCandType() {
    CandType result;
    sdp::string_to_cand_type(value, result);
    return result;
  }

  SetupType Token::toSetupType() {
    SetupType result;
    sdp::string_to_setup_type(value, result);
    return result;
  }

  /* ----------------------------------------------------------- */

  Line::Line() {
  }

  Line::Line(std::string src)
    :value(src)
    ,index(0)
  {
  }

  void Line::skip(char until) {
    for(;index < value.size(); ++index) {
      if (value[index] == until) {
        index++;
        break;
      }
    }
  }

  void Line::ltrim() {
    for(size_t i = index; i < value.size(); ++i) {
      if (value[i] == ' ') {
        index++;
      }
      else {
        break;
      }
    }
  }

  std::string Line::readString(char until) {
    Token t = getToken(until);
    if (t.size() == 0) {
      throw ParseException("Invalid Token. Token is empty: " +value);
      return "";
    }
    return t.toString();
  }

  int Line::readInt(char until) {

    Token t = getToken(until);

    if (t.size() == 0) {
      throw ParseException("Token is empty");
      return 0;
    }

    if (!t.isNumeric()) {
      throw ParseException("Token is not numeric.");
      return 0;
    }

    return t.toInt();
  }

  uint64_t Line::readU64(char until) {

    Token t = getToken(until);

    if (t.size() == 0) {
      throw ParseException("Token is empty");
      return 0;
    }

    if (!t.isNumeric()) {
      throw ParseException("Token is not numeric.");
      return 0;
    }

    return t.toU64();
  }

  /* SDP_IP4 or SDP_IP6 */
  AddrType Line::readAddrType(char until) {

    Token t = getToken(until);
    if (t.size() == 0) {
      throw ParseException("Token is empty");
      return SDP_ADDRTYPE_NONE;
    }
    
    AddrType result = t.toAddrType();
    if (result == SDP_ADDRTYPE_NONE) {
      throw ParseException("Invalid address type");
    }

    return result;
  }

  NetType Line::readNetType(char until) {

    Token t = getToken(until);
    if (t.size() == 0) {
      throw ParseException("Token is empty");
      return SDP_NETTYPE_NONE;
    }

    NetType result = t.toNetType();
    if (result == SDP_NETTYPE_NONE) {
      throw ParseException("Invalid nettype");
    }

    return result;
  }

  MediaProto Line::readMediaProto(char until) {

    Token t = getToken(until);
    if (t.size() == 0) {
      throw ParseException("Token is empty");
      return SDP_MEDIAPROTO_NONE;
    }

    MediaProto result = t.toMediaProto();
    if (result == SDP_MEDIAPROTO_NONE) {
      throw ParseException("Invalid media proto");
    }

    return result;
  }

  MediaType Line::readMediaType(char until) {

    Token t = getToken(until);
    if (t.size() == 0) {
      throw ParseException("Token is empty");
      return SDP_MEDIATYPE_NONE;
    }

    MediaType result = t.toMediaType();
    if (result == SDP_MEDIATYPE_NONE) {
      throw ParseException("Invalid media type");
    }

    return result;
  }

  CandType Line::readCandType(char until) {

    Token t = getToken(until);
    if (t.size() == 0) {
      throw ParseException("Token is empty");
      return SDP_CANDTYPE_NONE;
    }

    CandType result = t.toCandType();
    if (result == SDP_CANDTYPE_NONE) {
      throw ParseException("Invalid candidate type: " +t.value);
    }

    return result;
  }

  SetupType Line::readSetupType(char until) {
    Token t = getToken(until);
    if (t.size() == 0) {
      throw ParseException("Token is empty");
      return SDP_SETUPTYPE_NONE;
    }

    SetupType result = t.toSetupType();
    if (result == SDP_SETUPTYPE_NONE) {
      throw ParseException("Invalid setup type: " +t.value);
    }

    return result;
  }

  Token Line::getToken(char until) {

    std::string result;

    for (size_t i = index; i < value.size(); ++i) {
      index++;
      if (value[i] == until) {
        break;
      }
      result.push_back(value[i]);
    }

    return Token(result);
  }

  bool Line::readType(char type) {
    if (value[0] == type) {
      skip('=');
      return true;
    }
    return false;
  }

  char Line::operator[](unsigned int dx) {
    return value.at(dx);
  }

  /* ----------------------------------------------------------- */

  int Reader::parse(std::string source, SDP* result) {

    if (!source.size()) {  
      return -1;  
    }

    std::vector<std::string> lines;    
    if (tokenize(source, '\n', lines) < 0) {
      return -2;
    }

    Node* parent = result; 

    for (size_t i = 0; i < lines.size(); ++i) {

      Line line(lines[i]);
      Node* node = parseLine(line);

      if (!node) {
        //printf("Error: cannot parse line: %ld\n", i);
        continue;
      }
      
      if (node->type == SDP_MEDIA) {
        result->add(node);
        parent = node;
      }
      else {
        parent->add(node);
      }
    }

    return 0;
  }

  Node* Reader::parseLine(Line& l) {

    switch(l[0]) {

      case 'v': { return parseVersion(l);             }
      case 'o': { return parseOrigin(l);              }
      case 's': { return parseSessionName(l);         }
      case 'i': { return parseSessionInformation(l);  }
      case 'u': { return parseURI(l);                 }
      case 'e': { return parseEmailAddress(l);        }
      case 'p': { return parsePhoneNumber(l);         }
      case 'c': { return parseConnectionData(l);      }
      case 't': { return parseTiming(l);              }
      case 'm': { return parseMedia(l);               }
      case 'a': { return parseAttribute(l);           }

      /* unhandled line */
      default: {
        printf("sdp: ERROR: unhandled line: %s\n", l.value.c_str());
        return NULL;
      }
    }

    return NULL;
  }

  /* v= */
  Version* Reader::parseVersion(Line& line) {

    if (!line.readType('v')) {   
      return NULL;  
    }

    Version* node = new Version();

    try {
      node->version = line.readInt();
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  };

  /* o= */
  Origin* Reader::parseOrigin(Line& line) {

    if (!line.readType('o')) {
      return NULL;
    }

    Origin* node = new Origin();
    
    try {
      node->username         = line.readString();     /* e.g. "roxlu", "-" */
      node->sess_id          = line.readString();     /* e.g. "621762799816690644" */
      node->sess_version     = line.readU64();        /* e.g. 1 */
      node->net_type         = line.readNetType();    /* e.g. SDP_IN */
      node->addr_type        = line.readAddrType();   /* e.g. SDP_IP4 */
      node->unicast_address  = line.readString();     /* e.g. "127.0.0.1" */ 
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  }

  /* s= */
  SessionName* Reader::parseSessionName(Line& line) {

    if (!line.readType('s')) {
      return NULL;
    }

    SessionName* node = new SessionName();
    
    try {
      node->session_name = line.readString();
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  }

  /* i= */
  SessionInformation* Reader::parseSessionInformation(Line& line) {

    if (!line.readType('i')) {
      return NULL;
    }

    SessionInformation* node = new SessionInformation();

    try {
      node->session_description = line.readString();
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  }

  /* u= */
  URI* Reader::parseURI(Line& line) {

    if (!line.readType('u')) {
      return NULL;
    }

    URI* node = new URI();

    try {
      node->uri = line.readString();
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  }

  /* e= */
  EmailAddress* Reader::parseEmailAddress(Line& line) {

    if (!line.readType('e')) {
      return NULL;
    }

    EmailAddress* node = new EmailAddress();

    try {
      node->email_address = line.readString();
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  }

  /* p= */
  PhoneNumber* Reader::parsePhoneNumber(Line& line) {

    if (!line.readType('e')) {
      return NULL;
    }

    PhoneNumber* node = new PhoneNumber();

    try {
      node->phone_number = line.readString();
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  }

  ConnectionData* Reader::parseConnectionData(Line& line) {

    if (!line.readType('t')) {
      return NULL;
    }

    ConnectionData* node = new ConnectionData();

    try {
      node->net_type            = line.readNetType();
      node->addr_type           = line.readAddrType();
      node->connection_address  = line.readString();
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  }

  /* t= */
  Timing* Reader::parseTiming(Line& line) {

    if (!line.readType('t')) {
      return NULL;
    }

    Timing* node = new Timing();

    try {
      node->start_time = line.readU64();
      node->stop_time  = line.readU64();
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  }

  /* m= */
  Media* Reader::parseMedia(Line& line) {

    if (!line.readType('m')) {
      return NULL;
    }

    Media* node = new Media();

    try {
      node->media = line.readMediaType();
      node->port = line.readInt();
      node->proto = line.readMediaProto();
      node->fmt = line.readInt();
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      delete node;
      node = NULL;
    }

    return node;
  }

  /* a= */
  Attribute* Reader::parseAttribute(Line& line) {

    if (!line.readType('a')) {
      return NULL;
    }

    Attribute* node = NULL;
    std::string name;

    try {

      name = line.readString(':');
      line.ltrim();

      if (name == "rtcp") {
        AttributeRTCP* attr = new AttributeRTCP();
        node = (Attribute*) attr;
        attr->port = line.readInt();
        attr->net_type = line.readNetType();
        attr->addr_type = line.readAddrType();
        attr->connection_address = line.readString();
      }
      else if (name == "candidate") {
        AttributeCandidate* attr = new AttributeCandidate();
        node = (Attribute*) attr;
        attr->foundation = line.readString();
        attr->component_id = line.readInt();
        attr->transport = line.readString();
        attr->priority = line.readU64();
        attr->connection_address = line.readString();
        attr->port = line.readInt();
        line.skip(' '); /* "typ" */
        attr->cand_type = line.readCandType();
        /* @todo read the other elements! */
      }
      else if (name == "ice-ufrag") {
        node = new Attribute();
        node->name = name;
        node->value = line.readString();
        node->attr_type = SDP_ATTR_ICE_UFRAG;
      }
      else if (name == "ice-pwd") {
        node = new Attribute();
        node->name = name;
        node->value = line.readString();
        node->attr_type = SDP_ATTR_ICE_PWD;
      }
      else if (name == "ice-options") {
        node = new Attribute();
        node->name = name;
        node->value = line.readString();
        node->attr_type = SDP_ATTR_ICE_OPTIONS;
      }
      else if (name == "fingerprint") {
        AttributeFingerprint* attr = new AttributeFingerprint();
        node = (Attribute*)attr; 
        attr->name = name;
        attr->hash_func = line.readString();
        attr->fingerprint = line.readString();
      }
      else if (name == "setup") {
        AttributeSetup* attr = new AttributeSetup();
        node = (Attribute*) attr;
        attr->name = name;
        attr->role = line.readSetupType();
      }
      else {
        node = new Attribute();
        node->name = name;
        node->attr_type = SDP_ATTR_UNKNOWN;

        if (line.value.size() > line.index) {
          node->value = line.value.substr(line.index); 
        }
      }
    }
    catch(std::exception& e) {
      printf("Error: %s\n", e.what());
      if (node) {
        delete node;
        node = NULL;
      }
    }

    return node;
  }

} /* namespace sdp */
