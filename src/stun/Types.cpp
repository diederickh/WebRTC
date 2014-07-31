#include <stun/Types.h>

namespace stun {

  /* --------------------------------------------------------------------- */

  std::string attribute_type_to_string(uint32_t t) {
    switch (t) { 
      case STUN_ATTR_TYPE_NONE: { return "STUN_ATTR_TYPE_NONE"; }
      case STUN_ATTR_MAPPED_ADDR: { return "STUN_ATTR_MAPPED_ADDR"; }
      case STUN_ATTR_CHANGE_REQ: { return "STUN_ATTR_CHANGE_REQ"; }
      case STUN_ATTR_USERNAME: { return "STUN_ATTR_USERNAME"; }
      case STUN_ATTR_MESSAGE_INTEGRITY: { return "STUN_ATTR_MESSAGE_INTEGRITY"; }
      case STUN_ATTR_ERR_CODE: { return "STUN_ATTR_ERR_CODE"; }
      case STUN_ATTR_UNKNOWN_ATTRIBUTES: { return "STUN_ATTR_UNKNOWN_ATTRIBUTES"; }
      case STUN_ATTR_CHANNEL_NUMBER: { return "STUN_ATTR_CHANNEL_NUMBER"; }
      case STUN_ATTR_LIFETIME: { return "STUN_ATTR_LIFETIME"; }
      case STUN_ATTR_XOR_PEER_ADDR: { return "STUN_ATTR_XOR_PEER_ADDR"; }
      case STUN_ATTR_DATA: { return "STUN_ATTR_DATA"; }
      case STUN_ATTR_REALM: { return "STUN_ATTR_REALM"; }
      case STUN_ATTR_NONCE: { return "STUN_ATTR_NONCE"; }
      case STUN_ATTR_XOR_RELAY_ADDRESS: { return "STUN_ATTR_XOR_RELAY_ADDRESS"; }
      case STUN_ATTR_REQ_ADDRESS_FAMILY: { return "STUN_ATTR_REQ_ADDRESS_FAMILY"; }
      case STUN_ATTR_EVEN_PORT: { return "STUN_ATTR_EVEN_PORT"; }
      case STUN_ATTR_REQUESTED_TRANSPORT: { return "STUN_ATTR_REQUESTED_TRANSPORT"; }
      case STUN_ATTR_DONT_FRAGMENT: { return "STUN_ATTR_DONT_FRAGMENT"; }
      case STUN_ATTR_XOR_MAPPED_ADDRESS: { return "STUN_ATTR_XOR_MAPPED_ADDRESS"; }
      case STUN_ATTR_RESERVATION_TOKEN: { return "STUN_ATTR_RESERVATION_TOKEN"; }
      case STUN_ATTR_PRIORITY: { return "STUN_ATTR_PRIORITY"; }
      case STUN_ATTR_USE_CANDIDATE: { return "STUN_ATTR_USE_CANDIDATE"; }
      case STUN_ATTR_PADDING: { return "STUN_ATTR_PADDING"; }
      case STUN_ATTR_RESPONSE_PORT: { return "STUN_ATTR_RESPONSE_PORT"; }
      case STUN_ATTR_SOFTWARE: { return "STUN_ATTR_SOFTWARE"; }
      case STUN_ATTR_ALTERNATE_SERVER: { return "STUN_ATTR_ALTERNATE_SERVER"; }
      case STUN_ATTR_FINGERPRINT: { return "STUN_ATTR_FINGERPRINT"; }
      case STUN_ATTR_ICE_CONTROLLED: { return "STUN_ATTR_ICE_CONTROLLED"; }
      case STUN_ATTR_ICE_CONTROLLING: { return "STUN_ATTR_ICE_CONTROLLING"; }
      case STUN_ATTR_RESPONSE_ORIGIN: { return "STUN_ATTR_RESPONSE_ORIGIN"; }
      case STUN_ATTR_OTHER_ADDRESS: { return "STUN_ATTR_OTHER_ADDRESS"; }
      default: { return "unknown"; } 
    }
  }

  std::string message_type_to_string(uint32_t t) {
    switch (t) {
      case STUN_BINDING_REQUEST: { return "STUN_BINDING_REQUEST"; }
      case STUN_BINDING_RESPONSE: { return "STUN_BINDING_RESPONSE"; }
      case STUN_BINDING_ERROR_RESPONSE: { return "STUN_BINDING_ERROR_RESPONSE"; }
      case STUN_BINDING_INDICATION: { return "STUN_BINDING_INDICATION"; }
      default: { return "unknown"; } 
    }
  }

  /* ----------------------------------------------------------- */

} /* namespace stun */
