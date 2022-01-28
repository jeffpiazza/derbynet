'use strict';

// Different objects involved in managing the timer communicate with each other
// via events.  Event handlers get registered in one static list, and an event
// gets sent to all registered handlers.  Events can also be scheduled for
// delivery at a future time, specified either by absolute time or milliseconds
// in the future.

class TimerEvent {
  /*
    IDENTIFIED
    PREPARE_HEAT_RECEIVED, args are roundid, heat, lanemask (all integers)
    MASK_LANES, lanemask as arg (resets the timer)
    ABORT_HEAT_RECEIVED,
    // GATE_OPEN and GATE_CLOSED may be repeatedly signaled
    GATE_OPEN,
    GATE_CLOSED,
    RACE_STARTED,
    RACE_FINISHED,
    // An OVERDUE event signals that expected results have not yet arrived
    LANE_RESULT, // Just one; args are lane and time (as strings)
    OVERDUE,
    // Eventually, overdue results give way to a GIVING_UP event,
    // which is roughly treated like another PREPARE_HEAT_RECEIVED.
    GIVING_UP,
    LANE_COUNT, // Some timers report how many lanes
    START_RACE,  // Remote start requested
    LOST_CONNECTION
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
    setTimeout(this.trigger.bind(this), delay, event, args);
  }

  static sendAt(when, event, args) {
    sendAfterMs(when - Date.now(), event, args);
  }
  
  static trigger(event, args) {
    if (Flag.debug_serial.value) {
      g_logger.debug_msg('trigger ' + event);
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
