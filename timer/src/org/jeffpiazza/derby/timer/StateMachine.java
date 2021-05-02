package org.jeffpiazza.derby.timer;

import jssc.SerialPortException;
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
  // If the gate state is not knowable, then GATE_OPENED and GATE_CLOSED events
  // are not expected, and the state machine effectively alternates between IDLE
  // and MARK states.  GIVING_UP event doesn't happen, either, because it
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

  public synchronized void onEvent(Event e, String[] args) {
    if (!gate_state_is_knowable) {
      if (e == Event.GATE_CLOSED || e == Event.GATE_OPEN) {
        unexpected(e);
      }
    }
    State initialState = currentState;
    switch (currentState) {
      case IDLE:
        switch (e) {
          case PREPARE_HEAT_RECEIVED:
            currentState = State.MARK;
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
            currentState = State.SET;
            break;
          case ABORT_HEAT_RECEIVED:
            currentState = State.IDLE;
            break;
          case PREPARE_HEAT_RECEIVED:
            currentState = unexpected(e, State.MARK);
            break;
          case RACE_FINISHED:
            if (!gate_state_is_knowable) {
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
            Event.queue(Event.RACE_STARTED);
            break;
          case RACE_STARTED:
            currentState = State.RUNNING;
            break;
          case GATE_CLOSED:
            currentState = State.SET;
            break;
          case RACE_FINISHED:
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

  public void setGateStateNotKnowable() {
    gate_state_is_knowable = false;
  }
}
