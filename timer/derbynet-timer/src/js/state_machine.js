'use strict';

// The state machine keeps track of when a race is running vs when cars are
// being staged or the track is idle.

class StateMachine {
  state = 'IDLE';

  // Not used for much
  gate_state_is_knowable = true;

  constructor(gate_state_is_knowable) {
    console.log('gate_state_is_knowable:', gate_state_is_knowable);
    this.gate_state_is_knowable = gate_state_is_knowable;
  }

  // first_gate_change_ms marks the time at which a first GATE_OPEN is received
  // while the gate is believed to be closed, or the first GATE_CLOSED is
  // received while the gate is believed to be open.  When a confirming event
  // occurs after Flags.min_gate_time (in ms) has passed, the believed gate state
  // changes to match.  If a conflicting GATE_OPEN/GATE_CLOSED event occurs (i.e.,
  // confirming the current believed state), first_gate_change_ms resets to zero.
  first_gate_change_ms = 0;
  gate_is_believed_closed = false;

  // Returns true if the GATE_OPEN/GATE_CLOSED event should be interpreted as
  // conveying a new gate state.
  gateEventMarksAChange(event_says_closed) {
    if (event_says_closed == this.gate_is_believed_closed) {
      // The event just confirms what we already believed, overriding any
      // recent conflicting events.
      this.first_gate_change_ms = 0;
    } else {
      // Issue #35: If we have an apparent state change, don't record it
      // until it's stayed in the new state for a minimum length of time.
      // (Original issue report involved a SmartLine timer.)
      var now = Date.now();
      if (this.first_gate_change_ms == 0) {  // First gate change event
        this.first_gate_change_ms = now;
      } else if (now - this.first_gate_change_ms > Flag.min_gate_time.value) {
        this.gate_is_believed_closed = event_says_closed;
        this.first_gate_change_ms = 0;
        return true;
      }
    }
    return false;
  }

  onEvent(event, args) {
    if (!this.gate_state_is_knowable) {
      if (event == 'GATE_CLOSED' || event == 'GATE_OPEN') {
        this.unexpected(event);
      }
    }

    var initial = this.state;
    switch (this.state) {
    case 'IDLE':
      switch (event) {
      case 'PREPARE_HEAT_RECEIVED':
        this.state = 'MARK';
        this.gate_is_believed_closed = false;
        break;
      case 'ABORT_HEAT_RECEIVED':
      case 'RACE_FINISHED':
      case 'GIVING_UP':
        this.state = this.unexpected(event, 'IDLE');
        break;
      }
      break;
    case 'MARK':
      switch (event) {
      case 'GATE_CLOSED':
        if (this.gateEventMarksAChange(true)) {
          this.state = 'SET';
        }
        break;
      case 'GATE_OPEN':
        this.gateEventMarksAChange(false);
        break;
      case 'ABORT_HEAT_RECEIVED':
        this.state = 'IDLE';
        break;
      case 'PREPARE_HEAT_RECEIVED':
        this.state = this.unexpected(event, 'MARK');
        break;
      case 'RACE_FINISHED':
        if (!this.gate_state_is_knowable) {
          this.state = 'IDLE';
          break;
        }
        // else intentional fall-through
      case 'GIVING_UP':
        this.state = this.unexpected(event, 'IDLE');
        break;
      }
      break;
    case 'SET':
      switch (event) {
      case 'ABORT_HEAT_RECEIVED':
        this.state = 'IDLE';
        break;
      case 'PREPARE_HEAT_RECEIVED':
        this.state = this.unexpected(event, 'SET');
        break;
      case 'GATE_OPEN':
        if (this.gateEventMarksAChange(false)) {
          TimerEvent.send('RACE_STARTED');
        }
        break;
      case 'GATE_CLOSED':
        this.gateEventMarksAChange(true);
        break;
      case 'RACE_STARTED':
        this.state = 'RUNNING';
        break;
      case 'RACE_FINISHED':
      case 'GIVING_UP':
        this.state = this.unexpected(event, 'IDLE');
        break;
      }
      break;
    case 'RUNNING':
      switch (event) {
      case 'RACE_FINISHED':
        this.state = 'IDLE';
        break;
      case 'ABORT_HEAT_RECEIVED':
        this.state = 'IDLE';
        break;
      case 'PREPARE_HEAT_RECEIVED':
        this.state = this.unexpected(event, 'MARK');
        break;
      case 'GATE_OPEN':
        this.state = 'RUNNING';
        break;
        // GIVING_UP takes us back to a MARK state; if the gate is closed,
        // that should transition immediately to SET.
      case 'GIVING_UP':
        this.state = 'MARK';
        break;
      }
    }

    if (initial != this.state) {
      Gui.set_racing_state(this.state, this.gate_state_is_knowable);
      if (this.state != 'SET') {
        this.first_gate_open_ms = 0;
      }
      console.log(initial + ' >--' + event + '--> ' + this.state);
      g_logger.internal_msg(initial + ' >--' + event + '--> ' + this.state);
    }
  }

  unexpected(event, next = this.state) {
    console.log('Unexpected: ' + this.state + ' >--' + event + '--> ' + next);
    // g_logger.internal_msg('Unexpected: ' + this.state + ' >--' + event + '--> ' + next);
    return next;
  }
}

