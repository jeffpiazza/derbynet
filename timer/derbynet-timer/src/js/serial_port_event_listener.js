'use strict';

// Listener for ipc messages on serial-ports channel,
// sent from the electron script, timer.js, when run as
// a stand-alone application.
function serial_port_event_listener(event, message) {
  console.log('Received event', event);
  console.log(message);
}
