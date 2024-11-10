'use strict';

class Flag {
  static _all_flags = [];

  static find(name) {
    return Flag._all_flags.find(f => f.name == name);
  }

  static apply_all() {
    for (var f of Flag._all_flags) {
      if (f.on_apply_fn) {
        f.on_apply_fn(f.value);
      }
    }
  }

  static ignore_place = new Flag(
    "ignore-place", false, "Discard any place indications from timer");
 
  static delay_reset_after_race = new Flag(
    "delay-reset-after-race", 10,
    "How long after race over before timer will be reset, default 10s.");

  static remote_start_starts_heat = new Flag(
    "remote-start-starts-heat", false,
    "Triggering a remote start gets counted as the start of the heat for DNFs");

  static newline_expected_ms = new Flag(
    "newline-expected-ms", 200,
    "After this many milliseconds, assume an unterminated line"
      + " from the timer is complete (0 = wait forever).");

  // clear_rts_dtr: Not specifically supported, but serial API port.open does take a flowcontrol parameter.

  // Issue #35: While staging and/or latching the gate before a heat, polling can sometimes detect a fleeting
  // "gate open" condition, but this should not be interpreted as the start of a heat unless the "gate open"
  // condition lasts more than some minimum time.
  //
  static min_gate_time = new Flag(
    "min-gate-time", 500, "Ignore gate transitions shorter than <milliseconds>");

  static no_gate_watcher = new Flag(
    "no-gate-watcher", false,
    "Disable interrogation of timer's gate state.");

  static reset_after_start = new Flag(
    "reset-after-start", 11,
    "Reset timer <nsec> seconds after heat start, default 11.  Set 0 for never.");

  static fasttrack_automatic_gate_release = new Flag(
    "fasttrack-automatic-gate-release", false, "FastTrack light tree and automatic gate release installed");

  static fasttrack_disable_laser_reset = new Flag(
    "fasttrack-disable-laser-reset", false, "FastTrack disable laser reset (LR) command");

  static dtr_gate_release = new Flag(
    "dtr-gate-release", false, "EXPERIMENTAL Offer remote start via DTR signal (SCI device or similar)")
    .on_apply(function(v) {
      if (g_timer_proxy) {
        if (v) {
          if (!(g_timer_proxy.remote_start instanceof DtrRemoteStart)) {
            g_timer_proxy.remote_start =
              new DtrRemoteStart(g_timer_proxy.port_wrapper, g_timer_proxy.remote_start);
          }
        } else {
          if (g_timer_proxy.remote_start instanceof DtrRemoteStart) {
            g_timer_proxy.remote_start = g_timer_proxy.remote_start.profile_remote_start;
          }
        }
      }
    });

  static debug_serial = new Flag(
    "debug-io", false, "Enable debugging for low-level serial communication");

  name;
  type;
  value;
  description;
  
  constructor(name, value, description) {
    this.name = name;
    this.value = value;
    this.type = (this.value === true || this.value === false) ? "bool"
      : Number.isInteger(this.value) ? "int"
      : "string";
    this.description = description;

    Flag._all_flags.push(this);
  }

  on_apply_fn;
  on_apply(fn) {
    this.on_apply_fn = fn;
    return this;
  }

  assign(v) {  // v is a string
    if (this.type == 'bool') {
      this.value = (v == true || v == 'true');
    } else if (this.type == 'int') {
      this.value = parseInt(v);
    } else {
      this.value = v;
    }

    if (this.on_apply_fn) {
      this.on_apply_fn(this.value);
    }
  }


  static sendFlagsMessage(poller) {
    var msg = {
      action: 'timer-message',
      message: 'FLAGS',
    };
    for (var i = 0; i < Flag._all_flags.length; ++i) {
      var flag = Flag._all_flags[i];
      msg['flag-' + flag.name] = flag.type + ":" + flag.value;
      msg['desc-' + flag.name] = flag.description;
    }
    console.log(poller, msg);
    poller.sendMessage(msg);
  }
}
