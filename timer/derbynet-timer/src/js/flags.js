'use strict';

class Flag {
  static _all_flags = [];

  static ignore_place = new Flag(
    "ignore-place", false, "Discard any place indications from timer");

  static delay_reset_after_race = new Flag(
    "delay-reset-after-race", 10,
    "How long after race over before timer will be reset, default 10s.");

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
    "Reset timer <nsec> seconds after heat start, default 11");

  static fasttrack_automatic_gate_release = new Flag(
    "fasttrack-automatic-gate-release", false, "FastTrack light tree and automatic gate release installed");

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
