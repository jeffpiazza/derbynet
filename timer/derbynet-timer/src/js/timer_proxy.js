'use strict';

class TimerProxy {
  port_wrapper;
  profile;
  gate_watch_detectors;

  roundid;
  heat;
  lastFinishTime;
  result;

  sm;  // State machine

  detected_lane_count;
  // If not zero, a deadline for expecting heat results
  overdue_time;

  // port_wrapper is expected to be already opened
  constructor(port_wrapper, profile) {
    this.port_wrapper = port_wrapper;
    this.profile = profile;
    // start is async and runs forever
    this.start();
  }

  async start() {
    await this.setup();

    try {
      while (true) {
        var state = this.sm.state;

        var commands = this.profile?.poll?.[state];
        if (commands) {
          await this.port_wrapper.writeCommandSequence(commands);
        }

        if (state == 'RUNNING' && this.overdue_time != 0 && Date.now() >= this.overdue_time) {
          TimerEvent.send('OVERDUE');
        }

        if (this.profile?.gate_watcher && state != 'RUNNING') {
          await this.poll_gate_once();
        }

        await new Promise(r => setTimeout(r, 50));
      }
    } finally {
      // These are global clean-ups that don't really belong here
      g_timer_proxy = null;
      $("#probe-button").prop('disabled', false);
      $("#profiles-list li").removeClass('probing chosen');
    }
  }

  async poll_gate_once() {
    var deadline = Date.now() + /*POLL_RESPONSE_DEADLINE_MS*/100;

    for (var i = 0; i < this.gate_watch_detectors; ++i) {
      this.gate_watch_detectors[i].activateUntil(deadline);
    }
    await this.port_wrapper.write(this.profile.gate_watcher.command);
    await this.port_wrapper.drainUntil(deadline);
  }

  async setup() {
    if (this.profile?.options?.eol !== undefined) {
      this.port_wrapper.eol = this.profile.options.eol;
    }
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
      for (var i = 0; i < this.proffile.setup_queries.length; ++i) {
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
    TimerEvent.register(this.sm);
    TimerEvent.register(this);
  }

  async onEvent(event, args) {
    if (this.profile.hasOwnProperty('on') && this.profile.on.hasOwnProperty(event)) {
      await this.port_wrapper.writeCommandSequence(this.profile.on[event]);
    }
    switch (event) {
    case 'PREPARE_HEAT_RECEIVED': {
      this.roundid = args[0];
      this.heat = args[1];
      var lanemask = args[2];
      var pause = Math.max(0,
                           this.lastFinishTime + Flag.delay_reset_after_race.value * 1000 - Date.now());
      TimerEvent.sendAfterMs(pause, 'MASK_LANES', [lanemask]);
      break;
    }
    case 'MASK_LANES':
      this.maskLanes(args[0]);
      break;
    case 'ABORT_HEAT_RECEIVED':
      this.lastFinishTime = 0;
      this.roundid = this.heat = 0;
      break;
    case 'START_RACE':
      // TODO FastTrack profile's remote start value is controlled by flag
      if (this.profile?.remote_start?.has_remote_start) {
        this.port_wrapper.write(this.profile.remote_start.command);
      }
      break;
    case 'RACE_STARTED':
      if (this.profile?.options?.max_running_time_ms) {
        this.overdue_time = Date.now() + this.profile.options.max_running_time_ms;
      }
      break;
    case 'RACE_FINISHED':
      this.lastFinishTime = Date.now();
      this.roundid = 0;
      this.heat = 0;
      this.result = null;
      break;
    case 'LANE_RESULT': {
      var lane_char = args[0].charCodeAt(0);
      // ASCII 48 is '0', 57 is '9', 65 is 'A', 97 is 'a'
      var lane = (49 <= lane_char && lane_char <= 57) ?
          lane_char - 49 + 1 :
          lane_char - 65 + 1;
      if (this.result != null) {
        var place = 0;
        if (args.length > 2 && args[2] != null && !args[2].isEmpty()) {
          // ASCII 33 is '!', signifying place
          place = args[2].charCodeAt(0) - 33 + 1;
        }
        var valid = this.result.setLane(lane, args[1], place);
        if (valid && this.result.isFilled()) {
          TimerEvent.send('RACE_FINISHED', [this.roundid, this.heat, this.result]);
        }
      }
      break;
    }
    case 'LANE_COUNT':
      this.detected_lane_count = args[0].charCountAt(0) - 49 + 1;
      break;
    case 'GATE_OPEN':
      break;
    case 'GATE_CLOSED':
      break;
    case 'OVERDUE':
      TimerEvent.sendAfterMs(/*GIVE_UP_AFTER_OVERDUE_MS=*/1000, 'GIVING_UP');
      break;
    case 'GIVING_UP':
      break;
    }
  }

  async maskLanes(lanemask) {
    this.result = new HeatResult(lanemask);
    if (this.profile.hasOwnProperty('heat_prep')) {
      if (this.profile.heat_prep.hasOwnProperty('unmask')) {
        await this.port_wrapper.write(this.profile.heat_prep.unmask);
        var nlanes = Math.max(this.detected_lane_count || 0, this.profile?.options?.max_lanes || 0);
        for (var lane = 0; lane < nlanes; ++lane) {
          if ((lanemask & (1 << lane)) == 0) {
            await this.port_wrapper.drain();
            await this.port_wrapper.write(
              this.profile.heat_prep.mask +
                String.fromCharCode(this.profile.heat_prep.lane.charCodeAt(0) + lane));
          }
        }
      }
      if (this.profile.heat_prep.hasOwnProperty('reset')) {
        await this.port_wrapper.drain();
        await this.port_wrapper.write(this.profile.heat_prep.reset);
      }
      this.port_wrapper.drain();
    }
  }
}
