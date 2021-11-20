'use strict';

// The host poller handles the communication with the web server.  Messages from
// the server get converted into timer events (e.g., PREPARE_HEAT_RECEIVED or
// ABORT_HEAT_RECEIVED).  The host poller also listens for several events that
// it then reports to the web server.

const HEARTBEAT_PACE = 500;

var g_host_poller;  // See initialization at bottom of file

class HostPoller {
  // This URL is shared with role_finder.
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
    console.log('HostPoller constructor');  // TODO
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
      g_logger.set_remote_logging( nodes[0].getAttribute('send') == 'true' );
    }
    if (response.getElementsByTagName("abort").length > 0) {
      g_logger.host_in('ABORT HEAT');
      $("#heat-prepared").text("* Heat Aborted *");
      TimerEvent.send('ABORT_HEAT_RECEIVED', []);
    }
    if ((nodes = response.getElementsByTagName("heat-ready")).length > 0) {
      var args = [parseInt(nodes[0].getAttribute('roundid')),
                  parseInt(nodes[0].getAttribute('heat')),
                  parseInt(nodes[0].getAttribute('lane-mask'))];
      if (g_logger.do_logging) {
        g_logger.host_in('Prepare heat ' + args.join(','));
      }
      TimerEvent.send('PREPARE_HEAT_RECEIVED', args);
    }
    if ((nodes = response.getElementsByTagName("remote-start")).length > 0) {
      g_logger.host_in('START RACE');
      TimerEvent.send('START_RACE', []);
    }
    if ((nodes = response.getElementsByTagName("assign-flag")).length > 0) {
      // attributes for each node: flag=no-gate-watcher, value=true
      for (var i = 0; i < nodes.length; ++i) {
        var name = nodes[i].getAttribute('flag');
        var v = nodes[i].getAttribute('value');
        g_logger.host_in('assign-flag ' + name + ' = ' + v);
        console.log('assign-flag flag=' + name + ', value=' + v);
        for (var j = 0; j < Flag._all_flags.length; ++j) {
          var flag = Flag._all_flags[j];
          if (flag.name != name) {
            continue;
          }
          console.log('   type=' + flag.type);
          if (flag.type == 'bool') {
            flag.value = (v == 'true');
          } else if (flag.type == 'int') {
            flag.value = parseInt(v);
          } else {
            flag.value = v;
          }
        }
      }
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



$(function() {
  if (!g_standalone) {
    g_host_poller = new HostPoller();
  }
});
