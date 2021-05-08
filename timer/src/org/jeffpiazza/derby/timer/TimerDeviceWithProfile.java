package org.jeffpiazza.derby.timer;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Timestamp;
import org.jeffpiazza.derby.devices.RemoteStartInterface;
import org.jeffpiazza.derby.devices.TimerDeviceBase;
import org.jeffpiazza.derby.devices.TimerResult;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public class TimerDeviceWithProfile extends TimerDeviceBase
    implements Event.Handler, RemoteStartInterface {
  public TimerDeviceWithProfile(SerialPortWrapper portWrapper, Profile profile) {
    super(portWrapper);
    this.profile = profile;

    portWrapper.setEndOfLine(profile.options.end_of_line);
  }

  @Override
  public void close() {
    Event.unregister(sm);
    Event.unregister(this);
    super.close();
  }

  private Profile profile;
  private StateMachine sm;
  private String timerIdentifier;
  private int detected_lane_count = 0;

  private long lastFinishTime = 0;
  private int lanemask;

  private ArrayList<ProfileDetector> gate_watch_detectors
      = new ArrayList<ProfileDetector>();

  private TimerResult result;

  ScheduledEventsQueue queue = new ScheduledEventsQueue();

  // If non-zero, marks the time at which to generate an OVERDUE event
  // if we're still in a RUNNING state.  Leaving the RUNNING state sets this
  // back to zero.
  private long overdueTime = 0;

  protected static final int PRE_PROBE_SETTLE_TIME_MS = 2000;
  protected static final int PROBER_RESPONSE_TIME_MS = 500;
  protected static final int COMMAND_DRAIN_MS = 100;
  protected static final int POLL_RESPONSE_DEADLINE_MS = 100;
  protected static final int GIVE_UP_AFTER_OVERDUE_MS = 1000;

  public void abortHeat() throws SerialPortException {
    Event.trigger(Event.ABORT_HEAT_RECEIVED);
  }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    lanemask = laneMask;
    if (lastFinishTime == 0 || profile.options.display_hold_time_ms == 0) {
      Event.trigger(Event.PREPARE_HEAT_RECEIVED);
    } else {
      // This will defer acting on the prepareHeat message until some
      // minimum amount of time after the last heat finished.
      queue.addAt(lastFinishTime + profile.options.display_hold_time_ms,
                  Event.PREPARE_HEAT_RECEIVED);
    }
  }

  @Override
  public boolean canBeIdentified() {
    return profile.prober != null;
  }

  @Override
  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(profile.portParams.baudRate,
                                   profile.portParams.dataBits,
                                   profile.portParams.stopBits,
                                   profile.portParams.parity,
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
      drainForMs(PRE_PROBE_SETTLE_TIME_MS);
    }

    long deadline = System.currentTimeMillis() + PROBER_RESPONSE_TIME_MS;
    portWrapper.write(profile.prober.probe);
    int pi = 0;
    Pattern p = Pattern.compile(profile.prober.response_patterns[pi++]);
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      has_ever_spoken = true;
      Matcher m = p.matcher(s);
      if (m.find()) {
        if (pi >= profile.prober.response_patterns.length) {
          timerIdentifier = s;
          return true;
        } else {
          p = Pattern.compile(profile.prober.response_patterns[pi++]);
        }
      }
    }
    return false;
  }

  protected void setUp() throws SerialPortException {
    for (Profile.Detector detector_config : profile.detectors) {
      portWrapper.registerDetector(new ProfileDetector(detector_config));
    }
    if (profile.gate_watcher != null) {
      for (Profile.Detector detector_config : profile.gate_watcher.detectors) {
        ProfileDetector detector = new ProfileDetector(detector_config, false);
        gate_watch_detectors.add(detector);
        portWrapper.registerDetector(detector);
      }
    }

    if (profile.setup != null) {
      sendCommandSequence(profile.setup);
    }

    for (Profile.Query query : profile.setup_queries) {
      for (Profile.Detector detector_config : query.detectors) {
        ProfileDetector detector = new ProfileDetector(detector_config, false);
        detector.activateFor(COMMAND_DRAIN_MS);
        portWrapper.registerDetector(detector);
      }
      portWrapper.write(query.command);
      drainForMs();
    }
  }

  protected void sendCommandSequence(Profile.CommandSequence commands)
      throws SerialPortException {
    for (String cmd : commands.commands) {
      portWrapper.write(cmd);
      drainForMs();
    }
  }

  protected void drainForMs(int ms) {
    long deadline = System.currentTimeMillis() + ms;
    while (portWrapper.next(deadline) != null) {
      has_ever_spoken = true;
    }
  }

  protected void drainForMs() {
    drainForMs(COMMAND_DRAIN_MS);
  }

  protected void maskLanes(int lanemask) {
    try {
      if (profile.heat_prep != null) {
        if (profile.heat_prep.unmask_command != null) {
          portWrapper.write(profile.heat_prep.unmask_command);
          int nlanes = Math.max(detected_lane_count, profile.options.max_lanes);
          for (int lane = 0; lane < nlanes; ++lane) {
            if ((lanemask & (1 << lane)) == 0) {
              drainForMs();
              portWrapper.write(profile.heat_prep.mask_command
                  + (char) (profile.heat_prep.first_lane + lane));
            }
          }
        }
        if (profile.heat_prep.reset_command != null) {
          drainForMs();
          portWrapper.write(profile.heat_prep.reset_command);
        }
        drainForMs();
      }
      result = new TimerResult(lanemask);
    } catch (SerialPortException ex) {
      LogWriter.stacktrace(ex);
    }
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return detected_lane_count;
  }

  // This method really belongs on TimerDeviceWithProfile
  public void onEvent(Event event, String[] args) {
    Profile.CommandSequence custom = profile.custom_handlers.get(event);
    if (custom != null) {
      try {
        sendCommandSequence(custom);
      } catch (SerialPortException ex) {
        LogWriter.stacktrace(ex);
      }
    }
    switch (event) {
      case PREPARE_HEAT_RECEIVED:
        maskLanes(lanemask);
        break;
      case ABORT_HEAT_RECEIVED:
        lastFinishTime = 0;
        roundid = heat = 0;
        break;
      case RACE_STARTED:
        if (profile.options.max_running_time_ms != 0) {
          overdueTime = System.currentTimeMillis()
              + profile.options.max_running_time_ms;
        }
        invokeRaceStartedCallback();
        break;
      case RACE_FINISHED:
        lastFinishTime = System.currentTimeMillis();
        invokeRaceFinishedCallback(roundid, heat, result.toArray());
        roundid = heat = 0;
        result = null;
        break;
      case LANE_RESULT: {
        char lane_char = args[0].charAt(0);
        int lane = ('1' <= lane_char && lane_char <= '9')
                   ? lane_char - '1' + 1
                   : lane_char - 'A' + 1;
        if (result != null) {
          if (args.length == 2 || args[2] == null || args[2].isEmpty()) {
            result.setLane(lane, args[1]);
          } else {
            result.setLane(lane, args[1], args[2].charAt(0) - '!' + 1);
          }
          if (result.isFilled()) {
            Event.trigger(Event.RACE_FINISHED);
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
        queue.addAfterMs(GIVE_UP_AFTER_OVERDUE_MS, Event.GIVING_UP);
        break;
      case GIVING_UP:
        break;
    }
  }

  public void poll() throws SerialPortException, LostConnectionException {
    long deadline = System.currentTimeMillis() + POLL_RESPONSE_DEADLINE_MS;

    if (sm == null) {
      sm = new StateMachine();
      Event.register(this);
      Event.register(sm);
    }

    queue.poll();

    if (sm.state() == StateMachine.State.RUNNING && overdueTime != 0
        && System.currentTimeMillis() >= overdueTime) {
      Event.trigger(Event.OVERDUE);
    }

    {
      Profile.CommandSequence seq = profile.custom_poll_actions.get(sm.state());
      if (seq != null) {
        sendCommandSequence(seq);
      }
    }

    if (profile.gate_watcher != null
        && sm.state() != StateMachine.State.RUNNING) {
      // Required in MARK in order to energize the gate switch and detect open-to-closed.
      // (Actually the sequence is: PREPARE_HEAT, close the gate (undetected), reset laser gate, detect gate now closed.)
      portWrapper.write(profile.gate_watcher.command);
      for (ProfileDetector detector : gate_watch_detectors) {
        detector.activateFor(POLL_RESPONSE_DEADLINE_MS);
      }
    }

    while (portWrapper.next(deadline) != null) {
      // Let the detectors run, and pick up any hanging lines
    }
  }

  @Override
  public String getTimerIdentifier() {
    return timerIdentifier;
  }

  @Override
  public boolean hasRemoteStart() {
    return profile.remote_start != null && profile.remote_start.has_remote_start;
  }

  @Override
  public void remoteStart() throws SerialPortException {
    portWrapper.write(profile.remote_start.command);
    drainForMs();
  }
}
