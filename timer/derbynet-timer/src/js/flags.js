'use strict';

class Flag {
  static _all_flags = [];

  static ignore_place = new Flag(
    "ignore-place", false, "Discard any place indications from timer");

  static delay_reset_after_race = new Flag(
    "delay-reset-after-race", 10,
    "How long after race over before timer will be reset, default 10s.");
/*
  static newline_expected_ms = new Flag(
    "newline-expected-ms", 200,
    "After this many milliseconds, assume an unterminated line"
      + " from the timer is complete (0 = wait forever).");

  static clear_rts_dtr = new Flag(
    "clear-rts-dtr", false, "EXPERIMENTAL Initially clear RTS and DTR lines on serial port by default.");

  // Issue #35: Reject gate state changes that don't last "reasonably" long.
  // To do that, don't record a gate state change until it's aged a bit.
  //
  static min_gate_time = new Flag(
    "min-gate-time", 0, "Ignore gate transitions shorter than <milliseconds>");

  static pace = new Flag(
    "pace", 0, "Simulation staging pace (seconds between heats)");

  static no_gate_watcher = new Flag(
    "no-gate-watcher", false,
    "Disable interrogation of timer's gate state.");

  static reset_after_start = new Flag(
    "reset-after-start", 10,
    "TheJudge: Reset timer <nsec> seconds after heat start, default 10");

  static skip_enhanced_format = new Flag(
    "skip-enhanced-format", false,
    "FastTrack: Don't attempt enhanced format command");
  static skip_read_features = new Flag(
    "skip-read-features", false, "FastTrack: Don't attempt reading features");
  static fasttrack_automatic_gate_release = new Flag(
    "fasttrack-automatic-gate-release", false, "FastTrack light tree and automatic gate release installed");
*/

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
