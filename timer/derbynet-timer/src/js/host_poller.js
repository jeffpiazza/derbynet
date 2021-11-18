'use strict';

// The host poller handles the communication with the web server.  Messages from
// the server get converted into timer events (e.g., PREPARE_HEAT_RECEIVED or
// ABORT_HEAT_RECEIVED).  The host poller also listens for several events that
// it then reports to the web server.

const HEARTBEAT_PACE = 500;

class HostPoller {
  static url = 'action.php';

  next_message_time = 0;
  confirmed = 1;  // TODO

  remote_start = false;

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

  offer_remote_start(v) {
    this.remote_start = v;
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
          if (results[i]?.place && !Flag.ignore_place.value) {
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
    msg['remote-start'] = this.remote_start ? 'YES' : 'NO';
    this.next_message_time = Date.now() + HEARTBEAT_PACE;
    $.ajax(HostPoller.url,
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
      $("#heat-prepared").text("* Heat Aborted *");
      console.log('-> abort heat');  // TODO
      TimerEvent.send('ABORT_HEAT_RECEIVED', []);
    }
    if ((nodes = response.getElementsByTagName("heat-ready")).length > 0) {
      console.log(nodes);
      $("#heat-prepared").text("Round " + nodes[0].getAttribute('roundid')
                               + " heat " + nodes[0].getAttribute('heat')
                               + " mask " + nodes[0].getAttribute('lane-mask'));
      // TODO Remove this
      console.log('-> heat-ready ', [parseInt(nodes[0].getAttribute('roundid')),
                                     parseInt(nodes[0].getAttribute('heat')),
                                     parseInt(nodes[0].getAttribute('lane-mask'))]);
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
      Flag.sendFlagsMessage(this);
    }
  }
}
