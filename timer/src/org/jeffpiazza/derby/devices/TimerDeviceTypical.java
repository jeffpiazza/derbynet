package org.jeffpiazza.derby.devices;

import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.Timestamp;

// Superclass for implementing a new timer device based on a RacingStateMachine.
//
// Subclasses need to implement:
//  probe()
//  getNumberOfLanes()
//  getSafeNumberOfLanes()
//  maskLanes()
//  interrogateGateIsClosed()
//  whileInState()
//  onTransition()
public abstract class TimerDeviceTypical
    extends TimerDeviceBase
    implements RacingStateMachine.TransitionCallback {
  protected RacingStateMachine rsm;

  // Keeps track of last known state of the gate
  protected boolean gateIsClosed;

  protected String timerIdentifier;

  protected TimerDeviceTypical(SerialPortWrapper portWrapper) {
    super(portWrapper);
    this.rsm = new RacingStateMachine(this);
  }

  public void prepareHeat(int roundid, int heat, int lanemask)
      throws SerialPortException {
    RacingStateMachine.State state = rsm.state();
    // No need to bother doing anything if we're already prepared for this heat.
    // Plus, if the race should actually start while we're re-masking the lanes,
    // there's a chance we'll lose data.
    if (this.roundid == roundid && this.heat == heat
        && (state == RacingStateMachine.State.MARK
            || state == RacingStateMachine.State.SET)) {
      LogWriter.trace("Ignoring redundant prepareHeat()");
      return;
    }

    prepare(roundid, heat);
    describeLaneMask(lanemask);
    maskLanes(lanemask);
    rsm.onEvent(RacingStateMachine.Event.PREPARE_HEAT_RECEIVED);
  }

  // Returns a non-zero lane count: either the actual number of lanes, if known,
  // or some upper bound on the number of supported lanes.
  public int getSafeNumberOfLanes() throws SerialPortException {
    return getNumberOfLanes();
  }

  public String getTimerIdentifier() { return timerIdentifier; }

  public String describeLaneMask(int lanemask) throws SerialPortException {
    StringBuilder sb = new StringBuilder("Heat prepared: ");
    for (int lane = 0; lane < getSafeNumberOfLanes(); ++lane) {
      if ((lanemask & (1 << lane)) != 0) {
        sb.append(lane + 1);
      } else {
        sb.append("-");
      }
    }
    return sb.toString();
  }

  protected void raceFinished(Message.LaneResult[] results)
      throws SerialPortException {
    rsm.onEvent(RacingStateMachine.Event.RESULTS_RECEIVED);
    invokeRaceFinishedCallback(roundid, heat, results);
    roundid = heat = 0;
  }

  public void abortHeat() throws SerialPortException {
    rsm.onEvent(RacingStateMachine.Event.ABORT_HEAT_RECEIVED);
    roundid = heat = 0;
  }

  protected void logOverdueResults() {
    String msg = Timestamp.string() + ": ****** Race timed out *******";
    LogWriter.serial(msg);
    System.err.println(msg);
  }

  public void poll() throws SerialPortException, LostConnectionException {
    RacingStateMachine.State state = rsm.state();
    // If the gate is already closed when a PREPARE_HEAT message was delivered,
    // the PREPARE_HEAT will have left us in a MARK state, but we need to
    // make a GATE_CLOSED event to move ahead to SET.
    if (state == RacingStateMachine.State.MARK && getGateIsClosed()) {
      rsm.onEvent(RacingStateMachine.Event.GATE_CLOSED);
      state = rsm.state();
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
        onGateStateChange(!closed);
      }
    }
  }

  protected synchronized void onGateStateChange(boolean nowClosed)
      throws SerialPortException {
    RacingStateMachine.State state = rsm.state();
    RacingStateMachine.State newState = rsm.onEvent(
        nowClosed ? RacingStateMachine.Event.GATE_CLOSED
        : RacingStateMachine.Event.GATE_OPENED);
    if (newState == RacingStateMachine.State.RUNNING && state != newState) {
      invokeRaceStartedCallback();
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
      LogWriter.serial("** Unable to determine starting gate state");
      System.err.println(Timestamp.string() + ": Unable to read gate state");
    }
    return getGateIsClosed();
  }

  // If the device supports it, mask out lanes that aren't being used.
  protected abstract void maskLanes(int lanemask) throws SerialPortException;

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

  // A reasonably common scenario is this: if the gate opens accidentally
  // after the PREPARE_HEAT, the timer starts but there are no cars to
  // trigger a result.  When that happens, some timers support a "force output"
  // command, which should be tried.  If not, or if it doesn't produce any
  // results, it's eventually necessary to give up and return to a MARK or
  // SET state.
  protected void giveUpOnOverdueResults() throws SerialPortException,
                                                 LostConnectionException {
    // This forces the state machine back to MARK.
    rsm.onEvent(RacingStateMachine.Event.GIVING_UP);
    // updateGateIsClosed() may throw a LostConnectionException if the
    // timer has become unresponsive; otherwise, we'll advance from
    // MARK to SET.
    if (updateGateIsClosed()) {
      // It can certainly happen that the gate gets closed while the race
      // is running.
      rsm.onEvent(RacingStateMachine.Event.GATE_CLOSED);
    }
    // TODO invokeMalfunctionCallback(false, "No result received from last heat.");
    // We'd like to alert the operator to intervene manually, but
    // as currently implemented, a malfunction(false) message would require
    // unplugging/replugging the timer to reset: too invasive.
    LogWriter.serial("No result from timer for the running race; giving up.");
  }
}
