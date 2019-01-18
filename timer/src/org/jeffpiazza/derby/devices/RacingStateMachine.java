package org.jeffpiazza.derby.devices;

import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;

// Despite differences between different timers, the basic state machine to
// model them is essentially the same.
public class RacingStateMachine {
  public enum State {
    // After discovery, or after a race finishes, the track is IDLE.
    IDLE,
    // After prepareHeat and lane mask has been applied: MARK
    MARK,
    // After lane mask applied and gate closes, get SET to race.
    SET,
    // When the gate then opens, the race is RUNNING.
    RUNNING,
    // If the race starts but no results received within maxRunningTimeLimit,
    // then transition to RESULTS_OVERDUE state.  Depending on the device, there
    // may or may not be "force results" or similar command to try.  If there
    // isn't, or it doesn't work, a GIVING_UP event should eventually be posted.
    RESULTS_OVERDUE;
  }

  // Detectable external events that may change the state we believe we're in.
  //
  // Note that there's no LOST_CONNECTION, because if that happens, we're
  // going to abandon the state machine and everything else altogether.
  public enum Event {
    PREPARE_HEAT_RECEIVED,
    ABORT_HEAT_RECEIVED,
    GATE_OPENED,
    GATE_CLOSED,
    RESULTS_RECEIVED,
    // Eventually, a "results overdue" condition leads to a GIVING_UP event,
    // which is roughly treated like another PREPARE_HEAT_RECEIVED.
    GIVING_UP;
  }

  public interface TransitionCallback {
    public void onTransition(State oldState, State newState)
        throws SerialPortException;
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

  private TransitionCallback transition_callback;
  private LogWriter logWriter;
  private long stateEnteredMillis = 0;

  public RacingStateMachine(boolean gate_state_is_knowable,
                            TransitionCallback transition_callback,
                            LogWriter logWriter) {
    this.gate_state_is_knowable = gate_state_is_knowable;
    this.transition_callback = transition_callback;
    this.logWriter = logWriter;
  }

  public RacingStateMachine(TransitionCallback transition_callback,
                            LogWriter logWriter) {
    this(true, transition_callback, logWriter);
  }

  // Number of milliseconds we've been in the current state
  public long millisInCurrentState() {
    return System.currentTimeMillis() - stateEnteredMillis;
  }

  // Maximum amount of time, in milliseconds, that we're allowed to remain in
  // the RUNNING state.  If 0, then there's no limit.
  private long maxRunningTimeLimit = 0;

  public void setMaxRunningTimeLimit(long maxRunningTimeLimit) {
    this.maxRunningTimeLimit = maxRunningTimeLimit;
  }

  private State currentState = State.IDLE;

  // Reads the current state.  May advance RUNNING to RESULTS_OVERDUE state
  // if we've been waiting too long.
  public synchronized State state()
      throws SerialPortException {
    if (currentState == State.RUNNING && maxRunningTimeLimit > 0
        && System.currentTimeMillis() > stateEnteredMillis + maxRunningTimeLimit) {
      currentState = State.RESULTS_OVERDUE;
      stateEnteredMillis = System.currentTimeMillis();
      if (transition_callback != null) {
        transition_callback.onTransition(State.RUNNING, currentState);
      }
    }
    return currentState;
  }

  public synchronized State onEvent(Event e)
      throws SerialPortException {
    if (!gate_state_is_knowable) {
      if (e == Event.GATE_CLOSED || e == Event.GATE_OPENED) {
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
          case RESULTS_RECEIVED:
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
          case RESULTS_RECEIVED:
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
          case GATE_OPENED:
            currentState = State.RUNNING;
            break;
          case ABORT_HEAT_RECEIVED:
            currentState = State.IDLE;
            break;
          case PREPARE_HEAT_RECEIVED:
          case GATE_CLOSED:
            currentState = unexpected(e, State.SET);
            break;
          case RESULTS_RECEIVED:
          case GIVING_UP:
            currentState = unexpected(e, State.IDLE);
            break;
        }
        break;
      case RUNNING:
      case RESULTS_OVERDUE:
        switch (e) {
          case RESULTS_RECEIVED:
            currentState = State.IDLE;
            break;
          case ABORT_HEAT_RECEIVED:
            currentState = State.IDLE;
            break;
          case PREPARE_HEAT_RECEIVED:
            currentState = unexpected(e, State.MARK);
            break;
          case GATE_OPENED:
            currentState = unexpected(e, State.RUNNING);
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
      if (transition_callback != null) {
        transition_callback.onTransition(initialState, currentState);
      }
      System.out.println(initialState + " >--" + e + "--> " + currentState);
      logWriter.serialPortLogInternal(
          initialState + " >--" + e + "--> " + currentState);
    }
    return currentState;
  }

  private void unexpected(Event e) {
    logWriter.serialPortLogInternal("Unexpected event: " + e);
  }

  private State unexpected(Event e, State next) {
    logWriter.serialPortLogInternal(
        "Unexpected transition: " + currentState + " >--" + e + "--> " + next);
    return next;
  }

  public void setGateStateNotKnowable() {
    gate_state_is_knowable = false;
  }
}
