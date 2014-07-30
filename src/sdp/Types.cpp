#include <sdp/Types.h>

namespace sdp {

  /* ----------------------------------------------------------- */

  /* generic sdp line */
  Node::Node(Type t)
    :type(t)
  {
  }

  Node::~Node() {

    std::vector<Node*>::iterator it = nodes.begin();
    while (it != nodes.end()) {
      delete *it;
      it = nodes.erase(it);
    }
  }

  void Node::add(Node* n) {
    nodes.push_back(n);
  }

  bool Node::find(Type t, std::vector<Node*>& result) {
    for (size_t i = 0; i < nodes.size(); ++i) {
      if (nodes[i]->type == t) {
        result.push_back(nodes[i]);
      }
    }
    return result.size();
  }

  bool Node::find(MediaType t, Media** m) {
    Media* media = NULL;
    *m = NULL;
    for (size_t i = 0; i < nodes.size(); ++i) {
      if (nodes[i]->type == SDP_MEDIA) {
        media = static_cast<Media*>(nodes[i]);
        if (media->media == t) {
          *m = media;
          return true;
        }
      }
    }
    return false;
  }

  bool Node::find(AttrType t, Attribute** a) {
    Attribute* attr;
    *a = NULL;
    for (size_t i = 0; i < nodes.size(); ++i) {
      if (nodes[i]->type == SDP_ATTRIBUTE) {
        attr = static_cast<Attribute*>(nodes[i]);
        if (attr->attr_type == t) {
          *a = static_cast<Attribute*>(nodes[i]);
          return true;
        }
      }
    }
    return false;
  }

  bool Node::find(AttrType t, std::vector<Attribute*>& result) {
    Attribute* attr;
    for (size_t i = 0; i < nodes.size(); ++i) {
      if (nodes[i]->type == SDP_ATTRIBUTE) {
        attr = static_cast<Attribute*>(nodes[i]);
        if (attr->attr_type == t) {
          result.push_back(attr);
        }
      }
    }
    return result.size();
  }

  void Node::remove(Type t) {
    Node* node;
    std::vector<Node*>::iterator it = nodes.begin();
    while (it != nodes.end()) {
      node = *it;
      if (node->type == t) {
        delete node;
        it = nodes.erase(it);
        continue;
      }
      ++it;
    }
  }

  void Node::remove(AttrType t) {
    Attribute* attr;
    Node* node;
    std::vector<Node*>::iterator it = nodes.begin();
    while (it != nodes.end()) {
      node = *it;
      if (node->type == SDP_ATTRIBUTE) {
        attr = static_cast<Attribute*>(node);
        if (attr->attr_type == t) {
          delete node;
          it = nodes.erase(it);
          continue;
        }
      }
      ++it;
    }
  }

  /* ----------------------------------------------------------- */

  /* v=0 */
  Version::Version()
    :version(0)
    ,Node(SDP_VERSION)
  {
  }

  /* o=- 621762799816690644 7 IN IP4 127.0.0.1 */
  Origin::Origin()
    :net_type(SDP_IN)
    ,addr_type(SDP_IP4)
    ,sess_version(1)
    ,Node(SDP_ORIGIN)
  {
  }

  /* s=- */
  SessionName::SessionName()
    :Node(SDP_SESSION_NAME)
  {
  }

  /* i= */
  SessionInformation::SessionInformation()
    :Node(SDP_SESSION_INFORMATION)
  {
  }

  /* u= */
  URI::URI() 
    :Node(SDP_URI)
  {
  }

  /* e= */
  EmailAddress::EmailAddress()
    :Node(SDP_EMAIL_ADDRESS)
  {
  }

  /* p= */
  PhoneNumber::PhoneNumber() 
    :Node(SDP_PHONE_NUMBER)
  {
  }

  /* c= */
  ConnectionData::ConnectionData()
    :net_type(SDP_IN)
    ,addr_type(SDP_IP4)
    ,Node(SDP_CONNECTION_DATA)
  {
  }

  /* t=0 0 */
  Timing::Timing()
    :start_time(0)
    ,stop_time(0)
    ,Node(SDP_TIMING)
  {
  }

  /* m= */
  Media::Media()
    :media(SDP_MEDIATYPE_NONE)
    ,port(0)
    ,proto(SDP_MEDIAPROTO_NONE)
    ,fmt(0)
    ,Node(SDP_MEDIA)
  {
  }

  /* a= */
  Attribute::Attribute()
    :attr_type(SDP_ATTRTYPE_NONE)
    ,Node(SDP_ATTRIBUTE)
  {
  }

  Attribute::Attribute(std::string name, std::string value, AttrType atype)
    :name(name)
    ,value(value)
    ,attr_type(atype)
    ,Node(SDP_ATTRIBUTE)
  {
  }

  /* a=rtcp: */
  AttributeRTCP::AttributeRTCP()
    :Attribute()
  {
    attr_type = SDP_ATTR_RTCP;
  }


  /* a=candidate: ... */
  AttributeCandidate::AttributeCandidate()
    :component_id(0)
    ,priority(0)
    ,port(0)
    ,rel_port(0)
    ,Attribute()
  {
    attr_type = SDP_ATTR_CANDIDATE;
  }
  
  /* a=fingerprint:... */
  AttributeFingerprint::AttributeFingerprint()
    :Attribute()
  {
    attr_type = SDP_ATTR_FINGERPRINT;
  }
  
  AttributeFingerprint::AttributeFingerprint(std::string hfunc, std::string fprint) 
    :hash_func(hfunc)
    ,fingerprint(fprint)
    ,Attribute()
  {
    attr_type = SDP_ATTR_FINGERPRINT;
  }

  /* a=setup: */
  AttributeSetup::AttributeSetup()
    :role(SDP_SETUPTYPE_NONE)
    ,Attribute()
  {
    attr_type = SDP_ATTR_SETUP;
  }

  AttributeSetup::AttributeSetup(SetupType role)
    :role(role)
    ,Attribute()
  {
    attr_type = SDP_ATTR_SETUP;
  }

  void AttributeSetup::makeActive() {
    role = SDP_ACTIVE;
  }

  void AttributeSetup::makePassive() {
    role = SDP_PASSIVE;
  }
};
