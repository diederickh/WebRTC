/* 

   Signaling
   ---------

   Experimental and basic implementation of a signaling websocket API. We're 
   starting this signal server with the idea that it will be embedded in a 
   custom application which wants to allow users to connect to it using WebRTC. 

   The signaling server uses 'rooms' keep state. Each room has things like 
   the SDP of the server, name, etc.

 */
#ifndef WEBRTC_SIGNALING_H
#define WEBRTC_SIGNALING_H

#include <string>
#include <vector>
#include <mongoose.h>
#include <signaling/Room.h>
#include <signaling/SignalingSettings.h>

namespace sig {

  class Signaling {

  public:
    Signaling();
    ~Signaling();
    int init(SignalingSettings cfg);
    int addRoom(Room* room);
    int start();
    int stop();

    Room* findRoom(std::string name);

    /* API */
    void handleJoin(struct mg_connection* conn);

  public:
    SignalingSettings settings;
    std::vector<Room*> rooms;
    struct mg_server* server;
    bool must_stop;
  };

} /* namespace sig */
#endif
