'use strict';

// Different objects involved in managing the timer communicate with each other
// via events.  Event handlers get registered in one static list, and an event
// gets sent to all registered handlers.  Events can also be scheduled for
// delivery at a future time, specified either by absolute time or milliseconds
// in the future.

class TimerEvent {
  /*
    IDENTIFIED
    PREPARE_HEAT_RECEIVED, args are roundid, heat, lanemask, lane_count, round (all integers), and class
    MASK_LANES, lanemask as arg (resets the timer)
    ABORT_HEAT_RECEIVED,
    // GATE_OPEN and GATE_CLOSED may be repeatedly signaled
    GATE_OPEN,
    GATE_CLOSED,
    RACE_STARTED,
    RACE_FINISHED, args are roundid, heat, HeatResult
    LANE_RESULT, // Just one; args are lane and time (as strings)
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
    setTimeout(this.trigger.bind(this), 0, event, args);
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

    if (event != 'GATE_OPEN' && event != 'GATE_CLOSED') {
      if (event == 'PREPARE_HEAT_RECEIVED') {
        Gui.show_event(event);
      } else if (event == 'RACE_FINISHED') {
        Gui.show_event(event + " id=" + args[0] + " heat=" + args[1] + " " + args[2]);
      } else {
        Gui.show_event(event + " " + (args || []).join(','));
      }
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
