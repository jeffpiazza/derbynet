<?php

// If _ws_trigger_port is configured, sends $raw_msg (an array) over the trigger
// port.  $raw_msg should include a 'url' field to identify the websocket server
// the message is for.
//
// _ws_trigger_port allows server-side access to the websocket-based message
// system, without having to set up a websocket.
function maybe_send_ws_trigger($raw_msg) {
  $websocket_port = read_raceinfo('_ws_trigger_port');
  if ($websocket_port) {
    $msg = json_encode($raw_msg);
    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if ($socket !== false) {
      // _ws_trigger_port values is (port) : (ipaddr)
      $websocket_port = explode(':', $websocket_port, 2);
      $result = socket_connect($socket, $websocket_port[1], $websocket_port[0]);
      if ($result !== false) $result = socket_write($socket, '@'.strval(strlen($msg))."\n".$msg);
      if ($result === false) {
        trigger_error("Socket error: ".socket_strerror(socket_last_error($socket)), E_USER_ERROR);
      }
      socket_close($socket);
      return true;
    } else {
      trigger_error("Unable to create socket: ".socket_strerror(socket_last_error()), E_USER_ERROR);
    }
  }
  return false;
}

?>
