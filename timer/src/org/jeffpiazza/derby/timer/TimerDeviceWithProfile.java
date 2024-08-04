package org.jeffpiazza.derby.timer;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.RuntimeCondition;
import org.jeffpiazza.derby.Timestamp;
import org.jeffpiazza.derby.devices.RemoteStartInterface;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.devices.TimerDeviceBase;
import org.jeffpiazza.derby.devices.TimerDeviceUtils;
import org.jeffpiazza.derby.devices.TimerResult;
import org.jeffpiazza.derby.serialport.DtrRemoteStart;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;

public class TimerDeviceWithProfile extends TimerDeviceBase
    implements Event.Handler {
  public TimerDeviceWithProfile(TimerPortWrapper portWrapper, Profile profile) {
    super(portWrapper);
    setProfile(profile);
  }

  private void setProfile(Profile profile) {
    this.profile = profile;

    portWrapper.setEndOfLine(profile.options.eol);

    if (Flag.dtr_gate_release.value()) {
      remote_start = new DtrRemoteStart(portWrapper);
    } else {
      remote_start
          = profile.remote_start == null ? null
            : new ProfileRemoteStart(portWrapper, profile.remote_start);
    }
  }

  @Override
  public String humanName() {
    return profile.name;
  }

  @Override
  public void close() {
    Event.unregister(sm);
    Event.unregister(this);
    super.close();
  }

  private Profile profile;
  private StateMachine sm;
  private RemoteStartInterface remote_start = null;
  private String timerIdentifier;
  private int detected_lane_count = 0;

  private long lastFinishTime = 0;
  private int lanemask;

  private ArrayList<ProfileDetector> gate_watch_detectors
      = new ArrayList<ProfileDetector>();

  private TimerResult result;

  // If non-zero, marks the time at which to generate an OVERDUE event
  // if we're still in a RUNNING state.  Leaving the RUNNING state sets this
  // back to zero.
  private long overdueTime = 0;

  protected static final int PRE_PROBE_SETTLE_TIME_MS = 2000;
  protected static final int PROBER_RESPONSE_TIME_MS = 500;
  protected static final int POLL_RESPONSE_DEADLINE_MS = 100;
  protected static final int GIVE_UP_AFTER_OVERDUE_MS = 1000;

  public void abortHeat() throws SerialPortException {
    Event.send(Event.ABORT_HEAT_RECEIVED);
  }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    prepare(roundid, heat);
    lanemask = laneMask;

    if (lastFinishTime == 0 || Flag.delay_reset_after_race.value() == 0) {
      Event.send(Event.PREPARE_HEAT_RECEIVED);
    } else {
      // This will defer acting on the prepareHeat message until some
      // minimum amount of time after the last heat finished.
      Event.sendAt(lastFinishTime + Flag.delay_reset_after_race.value() * 1000,
                   Event.PREPARE_HEAT_RECEIVED);
    }
  }

  @Override
  public boolean canBeIdentified() {
    return profile.prober != null;
  }

  @Override
  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(profile.params.baud,
                                   profile.params.data,
                                   profile.params.stop,
                                   profile.params.parity,
                                   !Flag.clear_rts_dtr.value(),
                                   !Flag.clear_rts_dtr.value())) {
      return false;
    }
    if (profile.prober != null && !doProbe()) {
      return false;
    }

    setUp();

    return true;
  }

  private boolean doProbe() throws SerialPortException {
    if (profile.prober.pre_probe != null) {
      sendCommandSequence(profile.prober.pre_probe);
      portWrapper.drainForMs(PRE_PROBE_SETTLE_TIME_MS);
    }

    long deadline = System.currentTimeMillis() + PROBER_RESPONSE_TIME_MS;
    portWrapper.write(profile.prober.probe);
    int pi = 0;
    Pattern p = Pattern.compile(profile.prober.responses[pi++]);
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      Matcher m = p.matcher(s);
      if (m.find()) {
        if (pi >= profile.prober.responses.length) {
          // The JitRacemaster timer includes escape characters in some of its
          // responses, which can cause trouble when encoded as XML, so just
          // filter them out.
          timerIdentifier = s.replace("\033", "");
          return true;
        } else {
          p = Pattern.compile(profile.prober.responses[pi++]);
        }
      }
    }
    return false;
  }

  protected void setUp() throws SerialPortException {
    // For those timers that have a setup_query for it,
    // need to be able to receive LANE_COUNT event
    Event.register(this);

    for (Profile.Detector detector_config : profile.matchers) {
      portWrapper.registerDetector(new ProfileDetector(detector_config));
    }
    if (profile.gate_watcher != null) {
      for (Profile.Detector detector_config : profile.gate_watcher.matchers) {
        ProfileDetector detector = new ProfileDetector(detector_config, false);
        gate_watch_detectors.add(detector);
        portWrapper.registerDetector(detector);
      }
    }

    if (profile.setup != null) {
      sendCommandSequence(profile.setup);
    }

    for (Profile.Query query : profile.setup_queries) {
      for (Profile.Detector detector_config : query.matchers) {
        ProfileDetector detector = new ProfileDetector(detector_config, false);
        detector.activateFor(TimerPortWrapper.COMMAND_DRAIN_MS);
        portWrapper.registerDetector(detector);
      }
      portWrapper.write(query.command);
      portWrapper.drainForMs();
    }
  }

  protected void sendCommandSequence(Profile.CommandSequence commands)
      throws SerialPortException {
    for (String cmd : commands.commands) {
      portWrapper.write(cmd);
      portWrapper.drainForMs();
    }
  }

  private boolean ok_to_poll = true;

  private synchronized void suspendPolling() {
    ok_to_poll = false;
  }

  private synchronized void resumePolling() {
    ok_to_poll = true;
  }

  private synchronized boolean okToPoll() {
    return ok_to_poll;
  }

  protected void maskLanes(int lanemask) {
    suspendPolling();
    try {
      if (profile.heat_prep != null) {
        if (profile.heat_prep.unmask != null) {
          portWrapper.drainForMs();
          portWrapper.write(profile.heat_prep.unmask);
          int nlanes = detected_lane_count == 0
                       ? profile.options.max_lanes
                       : detected_lane_count;
          for (int lane = 0; lane < nlanes; ++lane) {
            if ((lanemask & (1 << lane)) == 0) {
              portWrapper.drainForMs();
              portWrapper.write(profile.heat_prep.mask
                  + (char) (profile.heat_prep.lane + lane));
            }
          }
        }
        if (profile.heat_prep.reset != null) {
          portWrapper.drainForMs();
          portWrapper.write(profile.heat_prep.reset);
        }
        portWrapper.drainForMs();
      }
      result = new TimerResult(lanemask);
    } catch (SerialPortException ex) {
      LogWriter.stacktrace(ex);
    } finally {
      resumePolling();
    }
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return detected_lane_count;
  }

  public void onEvent(Event event, String[] args) {
    // Make sure a RACE_STARTED event advances us to RUNNING state, as polling
    // (at least) needs to stop while race is running.  (Issue #200.)
    if (sm == null) {
      sm = new StateMachine(profile.options.gate_state_is_knowable);
    }
    sm.onEvent(event, args);

    Profile.CommandSequence custom = profile.on.get(event);
    if (custom != null) {
      try {
        // Pack936 reported an issue with the 'rg' being sent too quickly after
        // gate opening, causing the previous heat's results to be sent again.
        // This 50ms pause should address that.
        portWrapper.drainForMs(50);
        sendCommandSequence(custom);
      } catch (SerialPortException ex) {
        LogWriter.stacktrace(ex);
      }
    }
    switch (event) {
      case PREPARE_HEAT_RECEIVED:
        // prepareHeat was called when the server sent its message; then
        // prepareHeat set a delayed event to apply the lane mask and reset the
        // timer
        maskLanes(lanemask);
        overdueTime = 0;  // Insurance, not necessary
        break;
      case ABORT_HEAT_RECEIVED:
        lastFinishTime = 0;
        roundid = heat = 0;
        break;
      case RACE_STARTED:
        if (Flag.reset_after_start.value() == 0) {
          overdueTime = 0;
        } else {
          overdueTime = System.currentTimeMillis()
              + Flag.reset_after_start.value() * 1000;
        }
        invokeRaceStartedCallback();
        break;
      case RACE_FINISHED:
        lastFinishTime = System.currentTimeMillis();
        if (result != null) {
          invokeRaceFinishedCallback(roundid, heat, result.toArray());
        }
        roundid = heat = 0;
        result = null;
        overdueTime = 0;
        break;

      case LANE_RESULT: {
        char lane_char = args[0].charAt(0);
        int lane = ('1' <= lane_char && lane_char <= '9')
                   ? lane_char - '1' + 1
                   : lane_char - 'A' + 1;
        String time = TimerDeviceUtils.zeroesToNines(args[1]);
        if (result != null) {
          boolean wasFilled = result.isFilled();
          if (args.length == 2 || args[2] == null || args[2].isEmpty()) {
            result.setLane(lane, time);
          } else {
            result.setLane(lane, time, args[2].charAt(0) - '!' + 1);
          }
          // Send just a single RACE_FINISHED event, even if we get some extra
          // results for masked-out lanes, etc.
          if (result.isFilled() && !wasFilled) {
            Event.send(Event.RACE_FINISHED);
          }
        }
        break;
      }
      case LANE_COUNT:
        detected_lane_count = args[0].charAt(0) - '1' + 1;
        break;
      case GATE_OPEN:
        break;
      case GATE_CLOSED:
        break;
      case OVERDUE:
        String msg = Timestamp.string() + ": ****** Race timed out *******";
        LogWriter.trace(msg);
        System.err.println(msg);
        Event.sendAfterMs(GIVE_UP_AFTER_OVERDUE_MS, Event.GIVING_UP);
        break;
      case GIVING_UP:
        break;
      case GATE_WATCHER_NOT_SUPPORTED:
        // For at least some FastTrack timers, it's a configuration option
        // whether the timer will report the starting gate position or not.
        // If it's not configured, then stop probing to avoid interference with
        // reported results.
        if (sm != null) {
          sm.setGateStateNotKnowable();
        }
        break;
      case PROFILE_UPDATED:
        this.sm = null;  // We'll make a new one next time through.
        // By convention, each different timer type has a Java class that derives
        // from TimerDevicWithProfile and that exposes a static profile() method
        // to manufacture a new instance of the Profile.
        Class<? extends TimerDevice> cl = getClass();
        try {
          Method m = cl.getMethod("profile");
          setProfile((Profile) m.invoke(cl));
          if (this.roundid != 0) {
            // Redo lane masking (esp. for Champ, which may need to send a new
            // 'rg' when stopping gate watcher)
            prepareHeat(this.roundid, this.heat, this.lanemask);
          }
        } catch (Throwable ex) {
          Logger.getLogger(cl.getName()).log(Level.SEVERE, null, ex);
        }
        break;
    }
  }

  public void poll() throws SerialPortException, LostConnectionException {
    if (sm == null || !okToPoll()) {
      return;
    }
    long deadline = System.currentTimeMillis() + POLL_RESPONSE_DEADLINE_MS;

    if (sm.state() == StateMachine.State.RUNNING && overdueTime != 0
        && System.currentTimeMillis() >= overdueTime) {
      // If we're in RUNNING state, then we won't have been polling for gate
      // state, and likely won't have had any contact from the timer.  This
      // noticeContact() call is to reset the wrapper's connection-check timer,
      // so we don't start assuming that we've lost contact with the timer when
      // gatewatcher polling resumes.
      portWrapper.noticeContact();
      overdueTime = 0;  // Only send one OVERDUE event
      Event.send(Event.OVERDUE);
    }

    {
      Profile.StatePoller poller = profile.poll.get(sm.state());
      if (poller != null && RuntimeCondition.evaluate(poller.condition)) {
        sendCommandSequence(poller.commands);
      }
    }

    // StateMachine.gate_state_is_knowable() checks the value of the
    // no_gate_watcher flag, so no need to explicitly test here.
    if (profile.gate_watcher != null && sm.gate_state_is_knowable()
        && sm.state() != StateMachine.State.RUNNING) {
      // Required in MARK in order to energize the gate switch and detect
      // open-to-closed.  (Actually the sequence is:
      // PREPARE_HEAT, close the gate (undetected), reset laser gate,
      // detect gate now closed.)
      for (ProfileDetector detector : gate_watch_detectors) {
        detector.activateFor(POLL_RESPONSE_DEADLINE_MS);
      }
      portWrapper.write(profile.gate_watcher.command);
    }

    while (portWrapper.next(deadline) != null) {
      // Let the matchers run, and pick up any hanging lines
    }
  }

  @Override
  public String getTimerIdentifier() {
    return timerIdentifier;
  }

  @Override
  public RemoteStartInterface getRemoteStart() {
    return remote_start;
  }
}
