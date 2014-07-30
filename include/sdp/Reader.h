#ifndef SDP_READER_H
#define SDP_READER_H

#include <string>
#include <sstream>
#include <exception>
#include <sdp/SDP.h>
#include <sdp/Types.h>

namespace sdp { 

  /* our parse exception */
  struct ParseException : public std::exception {
    std::string s;
    ParseException(std::string s):s(s) {}
    ~ParseException() throw() {}
    const char* what() const throw() { return s.c_str(); };
  };

  /* one element of a Line */
  class Token {
  public:
    Token();
    Token(std::string value);
    bool isNumeric();
    size_t size();

    /* conversion functions */
    int         toInt();
    uint64_t    toU64();
    std::string toString();
    AddrType    toAddrType();
    NetType     toNetType();
    MediaType   toMediaType();
    MediaProto  toMediaProto();
    CandType    toCandType();
    SetupType   toSetupType();

  public:
    std::string value;
  };

  /* a sdp line, e.g "a=rtcp:59976 IN IP4 192.168.0.194" */
  class Line {
  public:
    Line();
    Line(std::string src);

    /* generic parse functions */
    void skip(char until);                                  /* skip some characters until you find the given character. */
    void ltrim();                                           /* trim whitespace from left from current index. */
    Token getToken(char until = ' ');                       /* read part of a sdp line until the given character. */
    char operator[](unsigned int);

    /* read the next token as a specific type */
    bool        readType(char type);                        /* read until the type element (e.g. o=, v=, a=) and return true when the line is the given type. */
    std::string readString(char until = ' ');               /* read a string from the next token */
    int         readInt(char until = ' ');                  /* read an integer value from the next token */
    uint64_t    readU64(char until = ' ');                  /* read an integer (u64). */
    AddrType    readAddrType(char until = ' ');             /* read an AddrType */
    NetType     readNetType(char until = ' ');              /* read a NetType */
    MediaType   readMediaType(char until = ' ');            /* read a MediaType */
    MediaProto  readMediaProto(char until = ' ');           /* read a MediaProto */ 
    CandType    readCandType(char until = ' ');             /* read a CandType */ 
    SetupType   readSetupType(char until = ' ');            /* read a SetupType */

  public:
    std::string value;
    size_t index;                                           /* used to keep track until which character one has read. */
  };

  /* parses an SDP */
  class Reader {
  public:
    int parse(std::string source, SDP* result);

  private:
    Node*               parseLine(Line& line);    
    Version*            parseVersion(Line& line);                /* v= */
    Origin*             parseOrigin(Line& line);                 /* o= */
    SessionName*        parseSessionName(Line& line);            /* s= */
    SessionInformation* parseSessionInformation(Line& line);     /* i= */
    URI*                parseURI(Line& line);                    /* u= */
    EmailAddress*       parseEmailAddress(Line& line);           /* e= */
    PhoneNumber*        parsePhoneNumber(Line& line);            /* p= */
    ConnectionData*     parseConnectionData(Line& line);         /* c= */
    Timing*             parseTiming(Line& line);                 /* t= */
    Media*              parseMedia(Line& line);                  /* m= */
    Attribute*          parseAttribute(Line& line);              /* a= */
  };

}

#endif
