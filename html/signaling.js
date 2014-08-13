/* 
   Signaling
   ---------

   Connects to the signaling server, tries to find the room on the signaling server and
   receives the SDP for this room. 
   
*/
var Signaling = function() {

  var me = this;
  this.is_connected = false;         /* used to keep state of our connection */
  this.onjoin = null;                /* gets called when you joined a room; SDP is given back. */
  this.onconnect = null;             /* gets called when connected. */

  this.connect =  function(url) {

    this.url = url;

    this.conn = new WebSocket(url);

    this.conn.onopen = function(ev) {
      me.is_connected = true
      
      if (me.onconnect) {
        me.onconnect();
      }
    }

    this.conn.onclose = function(ev) {
      me.is_connected = false;
    }

    this.conn.onerror = function(ev) {
      console.error("onerror", ev);
    }

    this.conn.onmessage = function(ev) {
      var el = ev.data.split(' ');
      if (null != me.onjoin && el[0] == "sdp") {
        me.onjoin(ev.data.substring(4, ev.data.length));
      }
    }
  }

  this.join = function(room) {

    if (!room) { 
      return false;
    }

    if (!me.is_connected) {
      console.error("Not connected yet; cannot join.");
      return false;
    }

    this.conn.send("join " +room);
  }
};
