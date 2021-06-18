'use strict';

// The host poller handles the communication with the web server.  Messages from
// the server get converted into timer events (e.g., PREPARE_HEAT_RECEIVED or
// ABORT_HEAT_RECEIVED).  The host poller also listens for several events that
// it then reports to the web server.

const HEARTBEAT_PACE = 500;

class HostPoller {

  next_message_time = 0;
  confirmed = 1;  // TODO

  /* TODO: Messages:
    IDENTIFIED lane_count, timer, human, ident, options)
    MALFUNCTION
    FLAGS
  */
  constructor() {
    TimerEvent.register(this);
    this.sendMessage({action: 'timer-message',
                      message: 'HELLO'});
    this.heartbeat_loop();
  }

  async heartbeat_loop() {
    while (true) {
      if (Date.now() >= this.next_message_time) {
        this.sendMessage({action: 'timer-message',
                          message: 'HEARTBEAT',
                          confirmed: this.confirmed ? 1 : 0});
      }
      await new Promise(r => setTimeout(r, this.next_message_time - Date.now()));
    }
  }
  
  async onEvent(event, args) {
    switch (event) {
    case 'IDENTIFIED':
      this.sendMessage({action: 'timer-message',
                        message: 'IDENTIFIED',
                        // TODO lane_count, ident, options
                        timer: args[0],  // TODO No formal name
                        human: args[0],
                        ident: args[1]});
      break;
    case 'RACE_STARTED':
      this.sendMessage({action: 'timer-message',
                        message: 'STARTED'});
      break;
    case 'RACE_FINISHED': {  // roundid, heat, results
      var msg = {action: 'timer-message',
                 message: 'FINISHED'};
      var results = args[2].lane_results;
      for (var i = 0; i < results.length; ++i) {
        if (results[i]) {
          msg['lane' + (i + 1)] = results[i].time;
          if (results[i]?.place) {
            msg['place' + (i + 1)] = results[i].place;
          }
        }
      }
      console.log(msg);
      this.sendMessage(msg);
      break;
    }
    case 'GIVING_UP':
    }
  }

  sendMessage(msg) {
    this.next_message_time = Date.now() + HEARTBEAT_PACE;
    $.ajax('action.php',
           {type: 'POST',
            data: msg,
            success: this.decodeResponse.bind(this)});
  }

  decodeResponse(response) {
    response = response.documentElement;
    var nodes;
    if ((nodes = response.getElementsByTagName("remote-log")).length > 0) {
      // TODO setRemoteLogging ( parseBoolean? nodes[0].getAttribute('send') )
    }
    if (response.getElementsByTagName("abort").length > 0) {
      TimerEvent.send('ABORT_HEAT_RECEIVED', []);
    }
    if ((nodes = response.getElementsByTagName("heat-ready")).length > 0) {
      TimerEvent.send('PREPARE_HEAT_RECEIVED', [parseInt(nodes[0].getAttribute('roundid')),
                                                parseInt(nodes[0].getAttribute('heat')),
                                                parseInt(nodes[0].getAttribute('lane-mask'))]);
    }
    if ((nodes = response.getElementsByTagName("remote-start")).length > 0) {
      TimerEvent.send('START_RACE', []);
    }
    if ((nodes = response.getElementsByTagName("assign-flag")).length > 0) {
      // TODO
    }
    if ((nodes = response.getElementsByTagName("assign-port")).length > 0) {
      // TODO
    }
    if ((nodes = response.getElementsByTagName("assign-device")).length > 0) {
      // TODO
    }
    if ((nodes = response.getElementsByTagName("query")).length > 0) {
      // TODO: Wants a flags message
    }
  }
}
