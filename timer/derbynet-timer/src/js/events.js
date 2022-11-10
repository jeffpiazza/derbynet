'use strict';

// Different objects involved in managing the timer communicate with each other
// via events.  Event handlers get registered in one static list, and an event
// gets sent to all registered handlers.  Events can also be scheduled for
// delivery at a future time, specified either by absolute time or milliseconds
// in the future.

class TimerEvent {
  /*
    IDENTIFIED, profile_name, probed, timer_id, vid, pid
    PREPARE_HEAT_RECEIVED, args are roundid, heat, lanemask, lane_count, round (all integers), and class
    MASK_LANES, lanemask as arg (resets the timer)
    ABORT_HEAT_RECEIVED,
    // GATE_OPEN and GATE_CLOSED may be repeatedly signaled
    GATE_OPEN,
    GATE_CLOSED,
    RACE_STARTED,
    RACE_FINISHED, args are roundid, heat, HeatResult
    LANE_RESULT, lane (1-based), time (string), place (1-based or 0)
    // (LANE_RESULT is triggered with three strings and processed before promulgating.)
    //
    // Eventually, overdue results give way to a GIVING_UP event,
    // which is roughly treated like another PREPARE_HEAT_RECEIVED.
    GIVING_UP,  // Giving up on overdue results
    LANE_COUNT, // Some timers report how many lanes
    START_RACE,  // Remote start requested
    LOST_CONNECTION,
    GATE_WATCHER_NOT_SUPPORTED
  */

  static handlers = [];

  // handler is an object with onEvent(event, args)
  static register(handler) {
    this.handlers.push(handler);
  }
  static unregister(handler) {
    var index = this.handlers.indexOf(handler);
    if (index >= 0) {
      this.handlers.splice(index, 1);
    }
  }

  static send(event, args) {
    if (event === undefined) throw 'undefined event';
    if (Flag.debug_serial.value) {
      g_logger.debug_msg('send ' + event);
    }

    g_clock_worker.postMessage([null, 0, 'EVENT', event, args]);
  }

  static sendAfterMs(delay, event, args) {
    g_clock_worker.postMessage([event, delay, 'EVENT', event, args]);
  }

  static sendAt(when, event, args) {
    sendAfterMs(when - Date.now(), event, args);
  }
  
  static trigger(event, args) {
    if (Flag.debug_serial.value) {
      g_logger.debug_msg('trigger ' + event);
    }

    switch (event) {
    case 'LANE_RESULT': {
      var lane_char = args[0].charCodeAt(0);
      // ASCII 48 is '0', 57 is '9', 65 is 'A', 97 is 'a'
      var lane = (49 <= lane_char && lane_char <= 57) ?
          lane_char - 49 + 1 :
          lane_char - 65 + 1;
      var place = 0;
      if (args.length > 2 && args[2]) {
        // ASCII 33 is '!', signifying place
        place = args[2].charCodeAt(0) - 33 + 1;
      }

      args = [lane, zeroesToNines(args[1]), place];
    }
      break;

    case 'LANE_COUNT':
      args = [ args[0].charCodeAt(0) - 49 + 1 ];
      break;
    }

    for (var i = 0; i < this.handlers.length; ++i) {
      try {
        this.handlers[i].onEvent(event, args);
      } catch (error) {
        g_logger.stacktrace(error);
      }
    }
  }
}

function zeroesToNines(time) {
  if (time && time.match(/^0\.0+$/)) {
    return time.replaceAll('0', '9');
  }
  return time;
}
