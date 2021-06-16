'use strict';

// The state machine keeps track of when a race is running vs when cars are
// being staged or the track is idle.

class StateMachine {
  state = 'IDLE';

  // Neither of these are used for much
  gate_state_is_knowable = true;
  state_entered_ms = 0;

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
        this.state = 'SET';
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
        TimerEvent.send('RACE_STARTED');
        break;
      case 'RACE_STARTED':
        this.state = 'RUNNING';
        break;
      case 'GATE_CLOSED':
        this.state = 'SET';
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
      this.state_entered_ms = Date.now();
      console.log(initial + ' >--' + event + '--> ' + this.state);
    }
  }

  unexpected(event, next = this.state) {
    console.log('Unexpected: ' + this.state + ' >--' + event + '--> ' + next);
    return next;
  }
}

