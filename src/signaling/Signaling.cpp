#include <stdio.h>
#include <sstream>
#include <signaling/Signaling.h>

/* @todo - add SSL support to signaling */

namespace sig {

  /* -------------------------------------------------------------- */

  static void* signaling_server_thread(void* user);
  static int signaling_event_handler(struct mg_connection* conn, enum mg_event ev);

  /* ---------------------------------------------------------------- */

  Signaling::Signaling() 
    :server(NULL)
    ,must_stop(true)
  {
  }

  Signaling::~Signaling() {

    /* free the rooms */
    for (size_t i = 0; i < rooms.size(); ++i) {
      delete rooms[i];
    }

    rooms.clear();

    if (server) {
      mg_destroy_server(&server);
      server = NULL;
    }

    must_stop = true;
  }


  int Signaling::init(SignalingSettings cfg) {

    /* validate */
    if (0 == cfg.port.size()) {
      printf("Signaling::init() - error: invalid settings, no port given.\n");
      return -1;
    }

    /* initialize our mongoose based websocket server. */
    settings = cfg;

    return 0;
  }

  int Signaling::addRoom(Room* room) {
    if (!room) { return -1; } 

    rooms.push_back(room);

    return 0;
  }

  int Signaling::start() {

    /* validate */
    if (0 == settings.port.size()) {
      printf("Signaling::start() - error: cannot start signaling server because no port was set in the settings. Did you call init()?\n");
      return -1;
    }

    if (0 == rooms.size()) {
      printf("Signaling::start() - error: makes no sense to start withouth any rooms.\n");
      return -2;
    }

    must_stop = false;

    /* init mongoose */
    server = mg_create_server(this, signaling_event_handler); /* this will be set to mg_connection::server_param */
    if (!server) {
      printf("Signaling::start() - error: cannot create the mongoose server.\n");
      must_stop = true;
      return -3;
    }

    mg_set_option(server, "listening_port", settings.port.c_str());

    mg_start_thread(signaling_server_thread, this);

    return 0;
  }

  int Signaling::stop() {

    if (NULL == server) {
      printf("Signaling::stop() - error: cannot stop the signaling server because it seems we're not started yet.\n");
      return -1;
    }

    must_stop = true;

    return 0;
  }

  Room* Signaling::findRoom(std::string name) {
    for (size_t i = 0; i < rooms.size(); ++i) {
      if (rooms[i]->name == name) {
        return rooms[i];
      }
    }
    return NULL;
  }

  void Signaling::handleJoin(struct mg_connection* conn) {
    if (NULL == conn) { return ; } 
    if (conn->content_len < 5) { return ; } 

    /* extract the name of the room. */
    std::string data(conn->content + 5, conn->content_len - 5);
    std::stringstream ss(data);
    std::string room_name;
    std::string response;
    ss >> room_name;

    if (0 == room_name.size()) { 
      printf("Signaling::handleJoin() - error: invalid room name, empty.\n");
      return ;    
    }
    
    Room* room = findRoom(room_name);
    if (NULL == room) {
      printf("Signaling::handleJoin() - error; cannot find the room: `%s`\n", room_name.c_str());
      return;
    }

    response = "sdp " +room_name +" " +room->sdp;

    mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, response.c_str(), response.size());
  }

  /* -------------------------------------------------------------- */

  static int signaling_event_handler(struct mg_connection* conn, enum mg_event ev) {

    if (MG_REQUEST == ev) {

      if (conn->is_websocket) {

        /* join command */
        if (conn->content_len >= 5 && 0 == memcmp(conn->content, "join", 4)) {

          /* @todo handle invalid joins. */
          Signaling* s = static_cast<Signaling*>(conn->server_param);
          s->handleJoin(conn);
        }

        //return conn->content_len == 4 && !memcmp(conn->content, "exit", 4) ? MG_FALSE : MG_TRUE;
        return MG_TRUE;
      }
    } 
    else if (MG_AUTH == ev) {
      return MG_TRUE;
    } 

    return MG_FALSE;
  }

  static void* signaling_server_thread(void* user) {

    Signaling* s = static_cast<Signaling*>(user);

    while (false == s->must_stop) {
      mg_poll_server(s->server, 100);
    }

    mg_destroy_server(&s->server);
    s->server = NULL;

    return 0;
  }

} /* namespace sig */
