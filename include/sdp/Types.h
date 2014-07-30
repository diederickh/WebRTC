/*

  Types
  -----

  This file contains the structs/nodes that make up a SDP and is based on RFC4566.
  We follow the same naming as the elements as described in RFC4566.
  We implement the nodes in the same order as described in the "SDP specification"
  chapter in http://tools.ietf.org/html/rfc4566.html

 */

#ifndef SDP_TYPES_H
#define SDP_TYPES_H

#include <stdint.h>
#include <string>
#include <vector>

namespace sdp {

  enum Type {
    SDP_NONE,
    SDP_SESSION,                        /* a full SDP session */
    SDP_ORIGIN,
    SDP_VERSION,
    SDP_SESSION_NAME,
    SDP_SESSION_INFORMATION,
    SDP_URI,
    SDP_EMAIL_ADDRESS,
    SDP_PHONE_NUMBER,
    SDP_CONNECTION_DATA,
    SDP_TIMING,
    SDP_MEDIA,
    SDP_CANDIDATE,
    SDP_ATTRIBUTE
  };

  enum NetType {
    SDP_NETTYPE_NONE,
    SDP_IN
  };

  enum AddrType {
    SDP_ADDRTYPE_NONE,
    SDP_IP4,
    SDP_IP6
  };

  enum MediaType {
    SDP_MEDIATYPE_NONE,
    SDP_VIDEO,
    SDP_AUDIO,
    SDP_TEXT,
    SDP_APPLICATION,
    SDP_MESSAGE
  };

  enum MediaProto {
    SDP_MEDIAPROTO_NONE,
    SDP_UDP,
    SDP_RTP_AVP,
    SDP_RTP_SAVP,
    SDP_RTP_SAVPF                    /* http://tools.ietf.org/html/rfc5124 */
  };

  enum AttrType {
    SDP_ATTRTYPE_NONE,
    SDP_ATTR_RTCP,
    SDP_ATTR_KEYWDS,
    SDP_ATTR_TOOL,
    SDP_ATTR_PTIME,
    SDP_ATTR_MAXPTIME,
    SDP_ATTR_RTPMAP,
    SDP_ATTR_RECVONLY,
    SDP_ATTR_SENDRECV,
    SDP_ATTR_SENDONLY,
    SDP_ATTR_INACTIVE,
    SDP_ATTR_ORIENT,
    SDP_ATTR_TYPE,
    SDP_ATTR_CHARSET,
    SDP_ATTR_SDPLANG,
    SDP_ATTR_LANG,
    SDP_ATTR_CANDIDATE,
    SDP_ATTR_ICE_UFRAG,
    SDP_ATTR_ICE_PWD,
    SDP_ATTR_ICE_OPTIONS,
    SDP_ATTR_FINGERPRINT,
    SDP_ATTR_SETUP,
    
    /* etc... etc.. */
    SDP_ATTR_UNKNOWN  /* an generic attribute. different from SDP_ATTRTYPE_NONE as this one has been explicitly set by the user */
  };

  /* a=candidate: */
  enum CandType {
    SDP_CANDTYPE_NONE,
    SDP_HOST,
    SDP_SRFLX,
    SDP_PRFLX,
    SDP_RELAY
  };

  /* a=setup: */
  enum SetupType {
    SDP_SETUPTYPE_NONE,
    SDP_ACTIVE,
    SDP_PASSIVE,
    SDP_ACTPASS,
    SDP_HOLDCONN
  };

  /* forward declared for Node::find() */
  struct Version;
  struct Origin;
  struct SessionName;
  struct SessionInformation;
  struct Timing;
  struct ConnectionData;
  struct Media;
  struct Attribute;
  struct AttributeRTCP;
  struct AttributeCandidate;

  struct Node {
  public:
    Node(Type t);
    virtual ~Node();
    void add(Node* n);
    bool find(Type t, std::vector<Node*>& result);            /* will try to find a child node for the given type */
    bool find(MediaType t, Media** result);                   /* will set result to the first found media element of the given media type. */
    bool find(AttrType t, Attribute** result);                /* will set result to the first occurence of the attribute type. */
    bool find(AttrType t, std::vector<Attribute*>& result);   /* find all attributes for the given type. */
    void remove(Type t);                                      /* remove all nodes of the given type. */
    void remove(AttrType t);                                  /* remove attributes of this type. */

  public:
    Type type;
    std::vector<Node*> nodes;
  };

  /* v= */
  struct Version : public Node {
    Version();
    int version;
  };

  /* o= */
  struct Origin : public Node {
    Origin();

    std::string username;                /* users login, or "-" if you dont support user ids. */
    std::string sess_id;                 /* numeric string that is used as unique identifier, e.g. timestamp, e.g. "621762799816690644" */
    uint64_t sess_version;               /* version number of this SDP, e.g. "1"  */
    NetType net_type;                    /* SDP_IN */
    AddrType addr_type;                  /* SDP_IP4, SDP_IP6 */
    std::string unicast_address;         /* address of the machine from which the session was created, e.g. 127.0.0.1 */
  };

  /* s= */
  struct SessionName : public Node {
    SessionName();
    std::string session_name;
  };

  /* i= */
  struct SessionInformation : public Node {
    SessionInformation();
    std::string session_description;
  };

  /* u= */
  struct URI : public Node {
    URI();
    std::string uri;
  };

  /* e= */
  struct EmailAddress : public Node { 
    EmailAddress();
    std::string email_address;
  };

  /* p= */
  struct PhoneNumber : public Node {
    PhoneNumber();
    std::string phone_number;
  };

  /* t= */
  struct Timing : public Node {
    Timing();
    uint64_t start_time;
    uint64_t stop_time;
  };

  /* c= */
  struct ConnectionData : public Node {
    ConnectionData();
    NetType net_type;
    AddrType addr_type;
    std::string connection_address; 
  };

  /* m= */
  struct Media : public Node {
    Media();
    MediaType media;
    uint16_t port;
    MediaProto proto;
    int fmt;
  };

  /* 

     Because the list of attribute types is huge, we create a generic Attribute
     struct which contains some members that are meant for common types. So in general
     not all members of this sturct are always used.  The reader will set the members
     base on the AttrType member.

     a=
   */
  struct Attribute : public Node {
    Attribute();
    Attribute(std::string name, std::string value, AttrType atype = SDP_ATTR_UNKNOWN);
    AttrType attr_type;
    std::string name;
    std::string value;
  };

  /* a=rtcp:59976 IN IP4 192.168.0.194 */
  struct AttributeRTCP : public Attribute {
    AttributeRTCP();
    uint16_t port;
    NetType net_type;
    AddrType addr_type;
    std::string connection_address;
  };

  /* a=candidate:4252876256 1 udp 2122260223 192.168.0.194 59976 typ host generation 0 */                                                                                                                                                                                                                                                                                     
  struct AttributeCandidate : public Attribute {
    AttributeCandidate();

    std::string foundation;
    uint64_t component_id;
    std::string transport;
    uint64_t priority;
    std::string connection_address;
    int port;
    CandType cand_type;
    std::string rel_addr;
    uint16_t rel_port;    
  };

  /* a=fingerprint:sha-256 EA:A3:3E:F1:7F:36:62:AA:AE:31:ED:9E:B5:B6:FA:CE:56:8A:29:4C:A3:C5:F7:28:3D:1B:72:5A:68:9F:FE:33 */
  /* see: http://www.rfc-editor.org/rfc/rfc4572.txt, section 5 */
  struct AttributeFingerprint : public Attribute {
    AttributeFingerprint();
    AttributeFingerprint(std::string hfunc, std::string fprint);

    std::string hash_func;
    std::string fingerprint;
  };

  /* a=setup: ... */
  struct AttributeSetup : public Attribute {
    AttributeSetup();
    AttributeSetup(SetupType role);
    void makeActive();
    void makePassive();
    SetupType role;
  };

};

#endif
