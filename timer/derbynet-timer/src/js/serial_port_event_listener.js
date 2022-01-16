'use strict';


// https://www.electronjs.org/docs/latest/tutorial/devices#web-serial-api
//
// Listener for ipc messages on serial-ports channel,
// sent from the electron script, timer.js, when run as
// a stand-alone application.
function serial_port_event_listener(event, /* portlist */ selectedPort, requestedPorts) {
  console.log('serial-ports event', selectedPort, requestedPorts);
  g_logger.internal_msg('serial_port_event_listener');
}


const { ipcRenderer } = require('electron');
ipcRenderer.on('serial-ports', serial_port_event_listener);
