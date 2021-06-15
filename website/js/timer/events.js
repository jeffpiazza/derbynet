'use strict';

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
    LANE_COUNT // Some timers report how many lanes
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
    setTimeout(this.trigger.bind(this), 0, event, args);
  }

  static sendAfterMs(delay, event, args) {
    setTimeout(this.trigger.bind(this), delay, event, args);
  }

  static sendAt(when, event, args) {
    sendAfterMs(when - Date.now(), event, args);
  }
  
  static trigger(event, args) {
    for (var i = 0; i < this.handlers.length; ++i) {
      this.handlers[i].onEvent(event, args);
    }
  }
}
