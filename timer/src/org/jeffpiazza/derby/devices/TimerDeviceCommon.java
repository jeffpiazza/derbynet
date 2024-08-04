package org.jeffpiazza.derby.devices;

import jssc.SerialPortException;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.Timestamp;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;

public abstract class TimerDeviceCommon
    extends TimerDeviceBase
    implements RacingStateMachine.TransitionCallback {

  protected RacingStateMachine rsm;
  protected String timerIdentifier;

  protected TimerDeviceCommon(TimerPortWrapper portWrapper,
                              GateWatcher gateWatcher) {
    this(portWrapper, gateWatcher, true);
  }

  protected TimerDeviceCommon(TimerPortWrapper portWrapper,
                              GateWatcher gateWatcher,
                              boolean gate_state_is_knowable) {
    super(portWrapper);
    this.rsm = new RacingStateMachine(gate_state_is_knowable, this);
    this.gateWatcher = gateWatcher;

    if (Flag.no_gate_watcher.value()) {
      setGateStateNotKnowable();  // Sets gateWatcher to null
    }
  }

  // Several timers support lane masking with a "clear mask" command followed
  // by a "mask lane" command for each lane to be masked out.
  //
  // lanemask is the bit mask, with e.g. low-order zero bit indicating lane 1
  // will not be used.
  //
  // unmask_command is the command string to send to clear the mask, and
  // unmask_responses is the number of response lines expected from the command.
  //
  // mask_command followed by a one-character lane marker masks out one lane.
  // first_lane is the character identifying lane 1, typically either 'A' or '1'
  // mask_responses is the number of response lines expected from each mask
  // command.
  protected void doMaskLanes(int lanemask,
                             String unmask_command, int unmask_responses,
                             String mask_command, char first_lane,
                             int mask_responses)
      throws SerialPortException {
    portWrapper.writeAndDrainResponse(unmask_command, unmask_responses, 500);
    for (int lane = 0; lane < getSafeNumberOfLanes(); ++lane) {
      if ((lanemask & (1 << lane)) == 0) {
        portWrapper.writeAndDrainResponse(
            mask_command + (char) (first_lane + lane),
            mask_responses, 500);
      }
    }
  }

  // System time at which raceFinished was last called.
  private long lastFinishTime = 0;
  private int pendingLaneMask = 0;
  private boolean laneMaskIsPending = false;

  // If non-zero, then avoid resetting the timer display until this time
  protected long displayHoldUntilMillis() {
    return lastFinishTime == 0 ? 0
           : lastFinishTime + Flag.delay_reset_after_race.value() * 1000;
  }

  protected synchronized void maybeProcessPendingLaneMask()
      throws SerialPortException {
    if (laneMaskIsPending) {
      if (System.currentTimeMillis() >= displayHoldUntilMillis()) {
        describeLaneMask(pendingLaneMask);
        maskLanes(pendingLaneMask);
        laneMaskIsPending = false;
      }
    }
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

    synchronized (this) {
      pendingLaneMask = lanemask;
      laneMaskIsPending = true;
      maybeProcessPendingLaneMask();
    }

    rsm.onEvent(RacingStateMachine.Event.PREPARE_HEAT_RECEIVED);
  }

  // Returns a non-zero lane count: either the actual number of lanes, if known,
  // or some upper bound on the number of supported lanes.
  public int getSafeNumberOfLanes() throws SerialPortException {
    return getNumberOfLanes();
  }

  public String getTimerIdentifier() {
    return timerIdentifier;
  }

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
    lastFinishTime = System.currentTimeMillis();
    rsm.onEvent(RacingStateMachine.Event.RESULTS_RECEIVED);
    invokeRaceFinishedCallback(roundid, heat, results);
    roundid = heat = 0;
  }

  public void abortHeat() throws SerialPortException {
    rsm.onEvent(RacingStateMachine.Event.ABORT_HEAT_RECEIVED);
    lastFinishTime = 0;
    roundid = heat = 0;
  }

  protected void logOverdueResults() {
    String msg = Timestamp.string() + ": ****** Race timed out *******";
    LogWriter.trace(msg);
    System.err.println(msg);
  }

  public static abstract class GateWatcher {
    // Keeps track of last known state of the gate
    protected boolean gateIsClosed;
    protected TimerPortWrapper portWrapper;

    // Tracks the clock time when the state first appeared to change, or 0 if
    // the interrogated gate state hasn't changed.
    protected long timeOfFirstChange = 0;

    public GateWatcher(TimerPortWrapper portWrapper) {
      this.portWrapper = portWrapper;
    }

    protected synchronized boolean getGateIsClosed() {
      return gateIsClosed;
    }

    protected synchronized void setGateIsClosed(boolean gateIsClosed) {
      this.gateIsClosed = gateIsClosed;
    }

    // Returns true if timer responds to interrogation saying the starting
    // gate is closed.  Throws NoResponseException if query goes unanswered.
    protected abstract boolean interrogateGateIsClosed()
        throws NoResponseException, SerialPortException, LostConnectionException;

    // Calls interrogateGateIsClosed and updates gateIsClosed.  If there's no
    // response, does a connection check and just returns the last gate state.
    protected boolean updateGateIsClosed()
        throws SerialPortException, LostConnectionException {
      try {
        boolean isClosedNow = interrogateGateIsClosed();
        synchronized (this) {
          if (isClosedNow == gateIsClosed) {
            timeOfFirstChange = 0;
          } else {
            // Issue #35: If we have an apparent state change, don't record it
            // until it's stayed in the new state for a minimum length of time.
            // (Original issue report involved a SmartLine timer.)
            long now = System.currentTimeMillis();
            if (timeOfFirstChange == 0) {
              timeOfFirstChange = now;
            } else if (now - timeOfFirstChange > Flag.min_gate_time.value()) {
              gateIsClosed = isClosedNow;
              timeOfFirstChange = 0;
            }
          }
        }
      } catch (NoResponseException ex) {
        portWrapper.checkConnection();

        // Don't know, assume unchanged
        LogWriter.serial("** Unable to determine starting gate state");
        System.err.println(Timestamp.string() + ": Unable to read gate state");
      }
      return getGateIsClosed();
    }
  }

  protected GateWatcher gateWatcher = null;

  public void poll() throws SerialPortException, LostConnectionException {
    maybeProcessPendingLaneMask();

    maybeSynthesizeGateClosedEvent(
        gateWatcher != null && gateWatcher.getGateIsClosed());

    RacingStateMachine.State state = rsm.state();

    whileInState(rsm.state());

    if (gateWatcher != null && okToDoGateCheck(state)) {
      boolean closed = gateWatcher.getGateIsClosed();
      if (closed != gateWatcher.updateGateIsClosed()) {
        onGateStateChange(!closed);
      }
    }
  }

  protected boolean okToDoGateCheck(RacingStateMachine.State state) {
    // Don't check the gate state while a race is running, and only check
    // occasionally when we're idle; otherwise, check constantly while
    // waiting for a race to start.
    boolean doGateCheck = state != RacingStateMachine.State.RUNNING;
    if (doGateCheck && state == RacingStateMachine.State.IDLE) {
      // At IDLE, only need to check gate occasionally, to confirm the
      // connection still works
      doGateCheck = portWrapper.millisSinceLastCommand() > 500;
    }
    return doGateCheck;
  }

  protected void maybeSynthesizeGateClosedEvent(boolean gateIsClosed)
      throws SerialPortException {
    // If the gate is already closed when a PREPARE_HEAT message was delivered,
    // the PREPARE_HEAT will have left us in a MARK state, but we need to
    // make a GATE_CLOSED event to move ahead to SET.
    if (rsm.state() == RacingStateMachine.State.MARK && gateIsClosed) {
      rsm.onEvent(RacingStateMachine.Event.GATE_CLOSED);
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

  // If the device supports it, mask out lanes that aren't being used.
  protected abstract void maskLanes(int lanemask) throws SerialPortException;

  // Called each time through poll(), before attempting to check the gate state.
  //
  // TODO FastTrack uses this to send laser gate reset events while in MARK state,
  // and to force an immediate transition out of RESULTS_OVERDUE state.
  // All other timers only use this as a substitute for timers.
  protected abstract void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException;

  @Override
  // Note that poll() does a separate, explicit check for transition
  // into RUNNING state.
  public abstract void onTransition(RacingStateMachine.State oldState,
                                    RacingStateMachine.State newState)
      throws SerialPortException;

  // FastTrackLegacy is the only caller for this
  protected void setGateStateNotKnowable() {
    rsm.setGateStateNotKnowable();
    gateWatcher = null;
  }

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
    if (gateWatcher != null && gateWatcher.updateGateIsClosed()) {
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
