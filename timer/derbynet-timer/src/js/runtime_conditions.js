'use strict';

class RuntimeCondition {
  static fasttrack_supports_laser_reset_ = true;

  static fasttrack_poll_mark() {
    return !Flag.fasttrack_automatic_gate_release.value &&
      !Flag.no_gate_watcher.value &&
      !Flag.fasttrack_disable_laser_reset.value &&
      RuntimeCondition.fasttrack_supports_laser_reset_;
  }

  static fasttrack_has_automatic_gate_release() {
    return Flag.fasttrack_automatic_gate_release.value;
  }
}


$(function() {
  TimerEvent.register({
    onEvent: function(event, args) {
      if (event == 'FASTTRACK_NO_LASER_RESET') {
        RuntimeCondition.fasttrack_supports_laser_reset_ = false;
      }
    }
  });
});

