'use strict';

class RuntimeCondition {
  static fasttrack_poll_mark() {
    return !Flag.fasttrack_automatic_gate_release.value &&
      !Flag.no_gate_watcher.value;
  }

  static fasttrack_has_automatic_gate_release() {
    return Flag.fasttrack_automatic_gate_release.value;
  }
}
