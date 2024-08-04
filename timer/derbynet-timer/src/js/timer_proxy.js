'use strict';

// Debugging notes when working without a timer:
//  Click the "Start scanning" button, then
//   g_prober.give_up = true;
//
//  var pw = new PortWrapper(g_ports[0]);
//  pw.open({baud: 9600});
//  TimerProxy.create(pw, all_profiles()[1]);


class TimerProxy {
  port_wrapper;
  profile;
  remote_start;
  gate_watch_detectors;

  roundid;
  heat;
  lastFinishTime;
  result;

  sm;  // State machine

  detected_lane_count = 0;
  // If not zero, a deadline for expecting heat results
  overdue_time;

  static async create(port_wrapper, profile) {
    this.destroy();
    g_timer_proxy = new TimerProxy(port_wrapper, profile);

    // For those timers that have a setup_query, need to be able to receive
    // LANE_COUNT event
    TimerEvent.register_unique(g_timer_proxy);

    await g_timer_proxy.setup();
  }

  static destroy() {
    if (g_timer_proxy) {
      TimerEvent.unregister(g_timer_proxy);
      g_timer_proxy.teardown();  // Closes the serial port
      g_timer_proxy = null;
    }
  }
  
  // port_wrapper is expected to be already opened
  constructor(port_wrapper, profile) {
    this.port_wrapper = port_wrapper;
    this.profile = profile;
    this.remote_start =
      profile?.remote_start ? new RemoteStart(port_wrapper, profile.remote_start) : null;
    if (this.profile?.options?.eol !== undefined) {
      this.port_wrapper.eol = this.profile.options.eol;
    }
  }

  _queue_next_poll(poll_start) {
    // Poll again after a delay
    const kPollIntervalMs = 250;
    g_clock_worker.postMessage(['POLL_TIMER', poll_start + kPollIntervalMs - Date.now(), 'POLL_TIMER']);
  }

  has_remote_start() {
    return this.remote_start?.has_remote_start();
  }

  async poll_once() {
    // This is the main timer polling loop.  Transmissions initiated by the timer (most importantly,
    // heat results) are handled asynchronously with detectors registered on the port_wrapper.
    try {
      var poll_start = Date.now();
      var state = this.sm.state;

      var poller = this.profile?.poll?.[state];
      if (poller) {
        if (!poller.condition || RuntimeCondition[poller.condition]()) {
          var prev = g_logger.scope;
          g_logger.scope = "profile.poll";
          try {
            await this.port_wrapper.writeCommandSequence(poller.commands);
          } finally {
            g_logger.scope = prev;
          }
        }
      }

      if (state == 'RUNNING' && this.overdue_time != 0 && Date.now() >= this.overdue_time) {
        this.port_wrapper.noticeContact();
        this.overdue_time = 0;  // Can't queue up more than one OVERDUE event
        TimerEvent.sendAfterMs(/*GIVE_UP_AFTER_OVERDUE_MS=*/1000, 'OVERDUE');
      }

      if (this.profile?.gate_watcher && this.sm.gate_state_is_knowable &&
          state != 'RUNNING' && !Flag.no_gate_watcher.value) {
        this.port_wrapper.checkConnection();
        await this.poll_gate_once();
      }

      this._queue_next_poll(poll_start);
    } catch (error) {
      // These are global clean-ups that don't really belong here
      console.log('timer_proxy poll caught error', error);
      TimerProxy.destroy();
      $("#probe-button").prop('disabled', false);
      $("#profiles-list li").removeClass('probing chosen');
      $("#ports-list li").removeClass('probing chosen');

      throw error;
    }
    // Intentially not caught: "Reader is closed" thrown from PortWrapper
  }

  async poll_gate_once() {
    var deadline = Date.now() + /*POLL_RESPONSE_DEADLINE_MS*/100;

    var prev = g_logger.scope;
    g_logger.scope = "poll_gate_once";
    try {
      for (var i = 0; i < this.gate_watch_detectors.length; ++i) {
        this.gate_watch_detectors[i].activateUntil(deadline);
      }
      await this.port_wrapper.write(this.profile.gate_watcher.command);
      await this.port_wrapper.drainUntil(deadline);
    } finally {
      g_logger.scope = prev;
    }
  }

  async setup() {
    if (this.profile?.matchers) {
      for (var i = 0; i < this.profile.matchers.length; ++i) {
        this.port_wrapper.detectors.push(new Detector(this.profile.matchers[i]));
      }
    }

    this.gate_watch_detectors = [];
    if (this.profile.hasOwnProperty('gate_watcher')) {
      for (var i = 0; i < this.profile.gate_watcher.matchers.length; ++i) {
        var d = new Detector(this.profile.gate_watcher.matchers[i]);
        d.activateFor(-1);
        this.gate_watch_detectors.push(d);
        this.port_wrapper.detectors.push(d);
      }
    }

    if (this.profile.hasOwnProperty('setup')) {
      await this.port_wrapper.writeCommandSequence(this.profile.setup);
    }
    // TODO Save and restore port_wrapper.detectors, as these setup queries' detectors don't need to
    // live beyond this one-time use.
    if (this.profile.hasOwnProperty('setup_queries')) {
      for (var i = 0; i < this.profile.setup_queries.length; ++i) {
        for (var j = 0; j < this.profile.setup_queries[i].matchers.length; ++j) {
          var d = new Detector(this.profile.setup_queries[i].matchers[j]);
          d.activateFor(COMMAND_DRAIN_MS);
          this.port_wrapper.detectors.push(d);
        }
        await this.port_wrapper.write(this.profile.setup_queries[i].command);
        await this.port_wrapper.drain();
      }
    }

    this.sm = new StateMachine(this.profile.options.gate_state_is_knowable);

    // Start the polling loop:
    this._queue_next_poll(Date.now());
  }

  async teardown() {
    this.port_wrapper.close();
  }

  async onEvent(event, args) {
    // Make sure a RACE_STARTED event advances us to RUNNING state, as polling
    // (at least) needs to stop while race is running.  (Issue #200.)
    if (this.sm) {
      // A LANE_COUNT event may arise during probing, before we have a state machine.
      this.sm.onEvent(event, args);
    }

    if (this.profile.hasOwnProperty('on') && this.profile.on.hasOwnProperty(event)) {
      var prev = g_logger.scope;
      g_logger.scope = "event.on";
      try {
        // Pack936 reported an issue with the 'rg' being sent too quickly after
        // gate opening, causing the previous heat's results to be sent again.
        // This 50ms pause should address that.
        await this.port_wrapper.drain(50);
        await this.port_wrapper.writeCommandSequence(this.profile.on[event]);
      } finally {
        g_logger.scope = prev;
      }
    }
    switch (event) {
    case 'PREPARE_HEAT_RECEIVED': {
      this.overdue_time = 0;  // Insurance, not really necessary
      this.roundid = args[0];
      this.heat = args[1];
      var pause = Math.max(0,
                           this.lastFinishTime + Flag.delay_reset_after_race.value * 1000 - Date.now());
      TimerEvent.sendAfterMs(pause, 'MASK_LANES', [/*lanemask=*/args[2], /*lanes=*/args[3]]);
      break;
    }
    case 'MASK_LANES':
      this.maskLanes(args[0], args[1]);
      break;
    case 'ABORT_HEAT_RECEIVED':
      this.lastFinishTime = 0;
      this.roundid = this.heat = 0;
      break;
    case 'START_RACE':
      if (this.remote_start) {
        this.remote_start.remote_start();
      }
      break;
    case 'RACE_STARTED':
      if (Flag.reset_after_start.value == 0) {
        this.overdue_time = 0;
      } else {
        this.overdue_time = Date.now() + Flag.reset_after_start.value * 1000;
      }
      break;
    case 'RACE_FINISHED':
      this.lastFinishTime = Date.now();
      this.roundid = 0;
      this.heat = 0;
      this.result = null;
      this.overdue_time = 0;
      break;
    case 'LANE_RESULT': {
      if (this.result != null) {
        var was_filled = this.result.isFilled();
        var valid = this.result.setLane(/*lane*/args[0], /*time*/args[1], /*place*/args[2]);
        // Send just a single RACE_FINISHED event, even if we get some extra
        // results for masked-out lanes, etc.
        if (valid && this.result.isFilled() && !was_filled) {
          TimerEvent.send('RACE_FINISHED', [this.roundid, this.heat, this.result]);
        }
      }
      break;
    }
    case 'LANE_COUNT':
      this.detected_lane_count = args[0];
      break;
    case 'GATE_OPEN':
      break;
    case 'GATE_CLOSED':
      break;
    case 'OVERDUE':
      break;
    case 'LOST_CONNECTION':
      console.log('TimerProxy sees lost connection', this);
      TimerEvent.unregister(this);  // TODO
      break;
    }
  }

  async maskLanes(lanemask, lanes) {
    this.result = new HeatResult(lanemask);
    if (this.profile.hasOwnProperty('heat_prep')) {
      if (this.profile.heat_prep.hasOwnProperty('unmask')) {
        await this.port_wrapper.drain();
        await this.port_wrapper.write(this.profile.heat_prep.unmask);
        var nlanes = this.detected_lane_count == 0
                       ? this.profile?.options?.max_lanes || 0
                       : this.detected_lane_count;
        for (var lane = 0; lane < nlanes; ++lane) {
          if ((lanemask & (1 << lane)) == 0) {
            console.log('masking lane ' + lane + ': ' + this.profile.heat_prep.mask +
                        String.fromCharCode(this.profile.heat_prep.lane.charCodeAt(0) + lane));
            await this.port_wrapper.drain();
            await this.port_wrapper.write(
              this.profile.heat_prep.mask +
                String.fromCharCode(this.profile.heat_prep.lane.charCodeAt(0) + lane));
          }
        }
      }
      await this.resetForHeatPrep();
      this.port_wrapper.drain();
    }
  }

  async resetForHeatPrep() {
      if (this.profile.heat_prep.hasOwnProperty('reset')) {
        await this.port_wrapper.drain();
        await this.port_wrapper.write(this.profile.heat_prep.reset);
      }
  }
}
