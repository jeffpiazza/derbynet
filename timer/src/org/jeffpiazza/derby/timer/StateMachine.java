package org.jeffpiazza.derby.timer;

import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;

// Despite differences between different timers, the basic state machine to
// model them is essentially the same.
public class StateMachine implements Event.Handler {
  public enum State {
    // After discovery, or after a race finishes, the track is IDLE.
    IDLE,
    // After prepareHeat and lane mask has been applied: MARK
    MARK,
    // After lane mask applied and gate closes, get SET to race.
    SET,
    // When the gate then opens, the race is RUNNING.
    RUNNING;
  }

  // The basic state machine assumes it's possible to detect when the start
  // gate opens or closes, possibly by repeatedly polling the timer to determine
  // the start gate's state.  For some timers, though, we never know whether the
  // gate is opened or closed, and so have to modify our behavior accordingly.
  private boolean gate_state_is_knowable;
  // If the gate state is not knowable, the state machine effectively manufactures
  // its own (assumed) GATE_CLOSED to advance from MARK to SET state after
  // seeing a PREPARE_HEAT_RECEIVED.  No GATE_OPEN event occurs, so the RUNNING
  // state is never entered.  GIVING_UP event doesn't happen, either, because it
  // can arise only from a RESULTS_OVERDUE state.

  public StateMachine(boolean gate_state_is_knowable) {
    this.gate_state_is_knowable = gate_state_is_knowable;
  }

  public StateMachine() {
    this(true);
  }

  private long stateEnteredMillis = 0;

  // Number of milliseconds we've been in the current state
  public long millisInCurrentState() {
    return System.currentTimeMillis() - stateEnteredMillis;
  }

  private State currentState = State.IDLE;

  // Reads the current state.  May advance RUNNING to RESULTS_OVERDUE state
  // if we've been waiting too long.
  public synchronized State state() {
    return currentState;
  }


  // first_gate_change_ms marks the time at which a first GATE_OPEN is received
  // while the gate is believed to be closed, or the first GATE_CLOSED is
  // received while the gate is believed to be open.  When a confirming event
  // occurs after Flags.min_gate_time (in ms) has passed, the believed gate state
  // changes to match.  If a conflicting GATE_OPEN/GATE_CLOSED event occurs (i.e.,
  // confirming the current believed state), first_gate_change_ms resets to zero.
  private long first_gate_change_ms = 0;
  private boolean gate_is_believed_closed = false;

  // Returns true if the GATE_OPEN/GATE_CLOSED event should be interpreted as
  // conveying a new gate state.
  private boolean gateEventMarksAChange(boolean event_says_closed) {
    if (event_says_closed == gate_is_believed_closed) {
      // The event just confirms what we already believed, overriding any
      // recent conflicting events.
      first_gate_change_ms = 0;
    } else {
      // Issue #35: If we have an apparent state change, don't record it
      // until it's stayed in the new state for a minimum length of time.
      // (Original issue report involved a SmartLine timer.)
      long now = System.currentTimeMillis();
      if (first_gate_change_ms == 0) {  // First gate change event
        first_gate_change_ms = now;
      } else if (now - first_gate_change_ms > Flag.min_gate_time.value()) {
        gate_is_believed_closed = event_says_closed;
        first_gate_change_ms = 0;
        return true;
      }
    }
    return false;
  }

  public synchronized void onEvent(Event e, String[] args) {
    State initialState = currentState;
    switch (currentState) {
      case IDLE:
        switch (e) {
          case PREPARE_HEAT_RECEIVED:
            currentState = State.MARK;
            gate_is_believed_closed = false;
            // Issue 270: If !gate_state_is_knowable, advance to State.SET after
            // a short wait.  This fixes the issue of FastTrack timers that don't
            // support the RG (read gate) command not knowing when to stop sending
            // LR (laser reset) commands.
            if (!gate_state_is_knowable()) {
              Event.sendAfterMs(500, Event.GATE_CLOSED);
            }
            break;
          case ABORT_HEAT_RECEIVED:
          case RACE_FINISHED:
          case GIVING_UP:
            currentState = unexpected(e, State.IDLE);
            break;
        }
        break;
      case MARK:
        switch (e) {
          case GATE_CLOSED:
            if (gateEventMarksAChange(true)) {
              currentState = State.SET;
            }
            break;
          case GATE_OPEN:
            gateEventMarksAChange(false);
            break;
          case ABORT_HEAT_RECEIVED:
            currentState = State.IDLE;
            break;
          case PREPARE_HEAT_RECEIVED:
            currentState = unexpected(e, State.MARK);
            break;
          case RACE_FINISHED:
            if (!gate_state_is_knowable()) {
              currentState = State.IDLE;
              break;
            }
          // else intentional fall-through
          case GIVING_UP:
            currentState = unexpected(e, State.IDLE);
            break;
        }
        break;
      case SET:
        switch (e) {
          case ABORT_HEAT_RECEIVED:
            currentState = State.IDLE;
            break;
          case PREPARE_HEAT_RECEIVED:
            currentState = unexpected(e, State.SET);
            break;
          case GATE_OPEN:
            if (gateEventMarksAChange(false)) {
              Event.send(Event.RACE_STARTED);
            }
            break;
          case GATE_CLOSED:
            gateEventMarksAChange(true);
            break;
          case RACE_STARTED:
            currentState = State.RUNNING;
            break;
          case RACE_FINISHED:
            if (!gate_state_is_knowable()) {
              currentState = State.IDLE;
              break;
            }
            // else intentional fall-through
          case GIVING_UP:
            currentState = unexpected(e, State.IDLE);
            break;
        }
        break;
      case RUNNING:
        switch (e) {
          case RACE_FINISHED:
            currentState = State.IDLE;
            break;
          case ABORT_HEAT_RECEIVED:
            currentState = State.IDLE;
            break;
          case PREPARE_HEAT_RECEIVED:
            currentState = unexpected(e, State.MARK);
            break;
          case GATE_OPEN:
            currentState = State.RUNNING;
            break;
          // GIVING_UP takes us back to a MARK state; if the gate is closed,
          // that should transition immediately to SET.
          case GIVING_UP:
            currentState = State.MARK;
            break;
        }
    }
    if (initialState != currentState) {
      stateEnteredMillis = System.currentTimeMillis();
      System.out.println(initialState + " >--" + e + "--> " + currentState);
      LogWriter.serial(initialState + " >--" + e + "--> " + currentState);
    }
  }

  private void unexpected(Event e) {
    LogWriter.serial("Unexpected event: " + e);
  }

  private State unexpected(Event e, State next) {
    LogWriter.serial("Unexpected transition: "
        + currentState + " >--" + e + "--> " + next);
    return next;
  }

  public boolean gate_state_is_knowable() {
    return gate_state_is_knowable && !Flag.no_gate_watcher.value();
  }

  public void setGateStateNotKnowable() {
    gate_state_is_knowable = false;
  }
}
