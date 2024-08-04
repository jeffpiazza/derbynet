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

  // Set true between the start of an ajax call to the host and its completion.
  message_in_flight = false;
  // The Date.now() value at which the next heartbeat message should go if no
  // other message gets sent sooner.  If more than HEARTBEAT_PACE ms passes
  // after next_message_time with no heartbeat sent, we report being "overdue",
  // which means the timer events aren't running at full pace.
  next_message_time = 0;

  identified = false;
  confirmed = true;

  // If the host detects timer messages coming from two different sources, it
  // sends a <competing/> element in the xml response.  We'll show a
  // #competing-modal dialog box in that event, and set this flag to suppress
  // sending more messages.
  competing = false;

  remote_start = false;

  constructor() {
    TimerEvent.register_unique(this);
    this.sendMessage({action: 'timer-message',
                      message: 'HELLO',
                      interface: 'web',
                      build: g_version.branch + "-" + g_version.revision});
  }

  offer_remote_start(v) {
    this.remote_start = v;
  }

  async heartbeat() {
    if (!this.message_in_flight && Date.now() >= this.next_message_time) {
      if (!this.identified) {
        this.sendMessage({action: 'timer-message',
                          message: 'HEARTBEAT',
                          unhealthy: true});
      } else {
        this.sendMessage({action: 'timer-message',
                          message: 'HEARTBEAT',
                          confirmed: this.confirmed ? 1 : 0});
      }
    }
  }

  async onEvent(event, args) {
    switch (event) {
    case 'IDENTIFIED':
      this.identified = true;
      this.confirmed = args[1];
      this.sendMessage({action: 'timer-message',
                        message: 'IDENTIFIED',
                        // TODO lane_count, ident, options
                        timer: args[0],  // TODO No formal name
                        human: args[0],
                        confirmed: this.confirmed ? 1 : 0,
                        ident: args[2],
                        vid: args[3],
                        pid: args[4]
                       });
      break;
    case 'LANE_RESULT':
    case 'GATE_OPEN':
    case 'GATE_CLOSED':
      this.confirmed = true;
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

      g_logger.internal_msg('RACE_FINISHED: ' + JSON.stringify(msg));
      this.sendMessage(msg);
      break;
    }
    case 'LOST_CONNECTION':
      this.identified = false;
      this.sendMessage({action: 'timer-message',
                        message: 'MALFUNCTION',
                        detectable: args[0] ? 1 : 0,
                        error: args[1]});
      break;
    }
  }

  sendMessage(msg) {
    msg['remote-start'] = this.remote_start ? 'YES' : 'NO';
    var now = Date.now();
    if (now > this.next_message_time + HEARTBEAT_PACE && this.next_message_time != 0) {
      // The heartbeat loop above should be sending messages regularly, but
      // apparently some browsers slow down non-frontmost windows, allowing
      // arbitrary delays in responding to timeouts.
      msg['overdue'] = now - this.next_message_time;
    }
    if (msg?.message != 'HEARTBEAT') {
      console.log('sendMessage', msg);
    }
    if (this.competing) {
      this.next_message_time = now + HEARTBEAT_PACE;
      g_clock_worker.postMessage(['HEARTBEAT', HEARTBEAT_PACE, 'HEARTBEAT']);
      console.log('Squelching message to host');
    } else {
      this.message_in_flight = true;
      $.ajax(HostPoller.url,
             {type: 'POST',
              data: msg,
              // decodeResponse also clears message_in_flight and sets
              // next_message_time
              success: this.decodeResponse.bind(this),
              error: function() {
                console.error('sendMessage fails');
                this.message_in_flight = false;
                this.next_message_time = now + HEARTBEAT_PACE;
                g_clock_worker.postMessage(['HEARTBEAT', HEARTBEAT_PACE, 'HEARTBEAT']);
              }
             });
    }
  }

  decodeResponse(response) {
    this.message_in_flight = false;
    this.next_message_time = Date.now() + HEARTBEAT_PACE;
    g_clock_worker.postMessage(['HEARTBEAT', HEARTBEAT_PACE, 'HEARTBEAT']);

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
                  parseInt(nodes[0].getAttribute('lane-mask')),
                  parseInt(nodes[0].getAttribute('lanes')),
                  parseInt(nodes[0].getAttribute('round')),
                  nodes[0].getAttribute('class')];
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
        var flag = Flag.find(name);
        if (flag) {
          flag.assign(v);
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
    if ((nodes = response.getElementsByTagName("debug")).length > 0) {
      for (var i = 0; i < nodes.length; ++i) {
        console.log("<debug>");
        console.log(nodes[i].textContent);
      }
    }
    if ((nodes = response.getElementsByTagName("competing")).length > 0) {
      this.competing = true;
      show_modal("#competing-modal");
      if (g_prober) {
        g_prober.give_up = true;
      }
      TimerProxy.destroy();
      g_host_poller = null;

      for (var i = 0; i < nodes.length; ++i) {
        console.log("<competing>", nodes[i].textContent);
      }
    }
  }
}



$(function() {
  if (!g_standalone) {
    g_host_poller = new HostPoller();
  }
});
