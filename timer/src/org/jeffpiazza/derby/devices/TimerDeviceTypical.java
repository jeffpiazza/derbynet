package org.jeffpiazza.derby.devices;

import jssc.SerialPortException;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.Timestamp;

public abstract class TimerDeviceTypical
    extends TimerDeviceBase
    implements TimerDevice, RacingStateMachine.TransitionCallback {
  protected RacingStateMachine rsm;

  // Keeps track of last known state of the gate
  protected boolean gateIsClosed;

  protected TimerDeviceTypical(SerialPortWrapper portWrapper) {
    super(portWrapper);
    this.rsm = new RacingStateMachine(portWrapper.logWriter());
  }

  protected void raceFinished(Message.LaneResult[] results)
      throws SerialPortException {
    rsm.onEvent(RacingStateMachine.Event.RESULTS_RECEIVED, this);
    invokeRaceFinishedCallback(roundid, heat, results);
    roundid = heat = 0;
  }

  public void abortHeat() throws SerialPortException {
    rsm.onEvent(RacingStateMachine.Event.ABORT_HEAT_RECEIVED, this);
    roundid = heat = 0;
  }

  protected void logOverdueResults() {
    String msg = Timestamp.string() + ": ****** Race timed out *******";
    portWrapper.logWriter().traceInternal(msg);
    System.err.println(msg);
  }

  public void poll() throws SerialPortException, LostConnectionException {
    RacingStateMachine.State state = rsm.state(this);
    // If the gate is already closed when a PREPARE_HEAT message was delivered,
    // the PREPARE_HEAT will have left us in a MARK state, but we need to
    // make a GATE_CLOSED event to move ahead to SET.
    if (state == RacingStateMachine.State.MARK && getGateIsClosed()) {
      rsm.onEvent(RacingStateMachine.Event.GATE_CLOSED, this);
      state = rsm.state(this);
    }

    whileInState(state);

    // Don't check the gate state while a race is running, and only check
    // occasionally when we're idle; otherwise, check constantly while
    // waiting for a race to start.
    boolean doCheckGate = state != RacingStateMachine.State.RUNNING;
    if (doCheckGate && state == RacingStateMachine.State.IDLE) {
      // At IDLE, only need to check gate occasionally, to confirm the
      // connection still works
      doCheckGate = portWrapper.millisSinceLastCommand() > 500;
    }

    if (doCheckGate) {
      boolean closed = getGateIsClosed();
      if (closed != updateGateIsClosed()) {
        closed = !closed;
        RacingStateMachine.State newState = rsm.onEvent(
            closed ? RacingStateMachine.Event.GATE_CLOSED
            : RacingStateMachine.Event.GATE_OPENED, this);
        if (newState == RacingStateMachine.State.RUNNING && state != newState) {
          invokeRaceStartedCallback();
        }
      }
    }
  }

  protected synchronized boolean getGateIsClosed() {
    return gateIsClosed;
  }

  protected synchronized void setGateIsClosed(boolean gateIsClosed) {
    this.gateIsClosed = gateIsClosed;
  }

  // Calls interrogateGateIsClosed and updates gateIsClosed.  If there's no
  // response, does a connection check and just returns the last gate state.
  protected boolean updateGateIsClosed()
      throws SerialPortException, LostConnectionException {
    try {
      setGateIsClosed(interrogateGateIsClosed());
    } catch (NoResponseException ex) {
      checkConnection();

      // Don't know, assume unchanged
      portWrapper.logWriter().serialPortLogInternal(
          "** Unable to determine starting gate state");
      System.err.println(Timestamp.string() + ": Unable to read gate state");
    }
    return getGateIsClosed();
  }

  // Returns true if timer responds to interrogation saying the starting
  // gate is closed.  Throws NoResponseException if query goes unanswered.
  protected abstract boolean interrogateGateIsClosed()
      throws NoResponseException, SerialPortException, LostConnectionException;

  // Called each time through poll(), before attempting to check the gate state.
  protected abstract void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException;

  @Override
  // Note that poll() does a separate, explicit check for transition
  // into RUNNING state.
  public abstract void onTransition(RacingStateMachine.State oldState,
                                    RacingStateMachine.State newState)
      throws SerialPortException;
}
