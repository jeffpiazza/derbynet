package org.jeffpiazza.derby.devices;

import java.util.regex.Matcher;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.SerialPortWrapper;

// This device class supports older FastTrack devices that don't accept (i.e.,
// are deaf to) commands from the host.
//
// From http://www.microwizard.com/faq.html:
//
// I want to write my own software. What is the format of the timer's serial interface?
// A. The timer sends this string when all the cars have finished.
//
// A=1.234! B=2.345 C=3.456 D=4.567 E=0.000 F=0.000
//
// It puts an exclamation point after the winning time, with 2 spaces after
// every time (the exclamation point takes up a space). Also if you only have a
// 4 lane track the unused lanes (E and F in this example) will show 0.000
//
// Other characters:
//
// It sends an "@" sign whenever the reset switch is closed. Also it sends a
// <LF> and a <CR> (That is ASCII code 10 and 13) at the end of each string.
//
// The serial settings to view the timer results yourself are 9600 baud, 8 bits,
// no parity, 1 stop bit, no flow control. With these settings you should be
// able to see the results in hyperterminal.
public class OlderFastTrackDevice extends TimerDeviceTypical {  // TODO TimerDeviceBase

  public OlderFastTrackDevice(SerialPortWrapper portWrapper) {
    super(portWrapper);

    // Once started, we expect a race result within 10 seconds; we allow an
    // extra second before considering the results overdue.
    rsm.setMaxRunningTimeLimit(11000);
  }

  private boolean gateOpen = false;

  public static final int MAX_LANES = 6;

  public static String toHumanString() {
    return "Older FastTrack";
  }

  @Override
  public boolean canBeIdentified() {
    return false;
  }

  // Since the device doesn't listen to anything we say, we don't "probe" the
  // device at all, we just assume that we're talking to one.
  public boolean probe() throws SerialPortException {
    if (!portWrapper.port().setParams(SerialPort.BAUDRATE_9600,
                                      SerialPort.DATABITS_8,
                                      SerialPort.STOPBITS_1,
                                      SerialPort.PARITY_NONE,
                                      /* rts */ false,
                                      /* dtr */ false)) {
      return false;
    }

    setUp();
    return true;
  }

  protected void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
      public String apply(String line) throws SerialPortException {
        Matcher m = TimerDeviceUtils.matchedCommonRaceResults(line);
        if (m != null) {
          Message.LaneResult[] results
              = TimerDeviceUtils.extractResults(line, m.start(), m.end(),
                                                MAX_LANES);
          raceFinished(results);
          return line.substring(0, m.start()) + line.substring(m.end());
        } else {
          return line;
        }
      }
    });
    portWrapper.registerEarlyDetector(new SerialPortWrapper.Detector() {
      @Override
      public String apply(String s) throws SerialPortException {
        if (s.charAt(0) == '@') {
          raceStarted();
          return s.substring(1);
        } else {
          return s;
        }
      }
    });
  }

  // No lane masks!
  public void prepareHeat(int roundid, int heat, int lanemask)
      throws SerialPortException {
    prepare(roundid, heat);
    StringBuilder sb = new StringBuilder("Heat prepared: ");
    for (int lane = 0; lane < MAX_LANES; ++lane) {
      if ((lanemask & (1 << lane)) != 0) {
        sb.append(lane + 1);
      } else {
        sb.append("-");
      }
    }
    portWrapper.logWriter().serialPortLogInternal(sb.toString());
    rsm.onEvent(RacingStateMachine.Event.PREPARE_HEAT_RECEIVED, this);
  }

  public void abortHeat() throws SerialPortException {
    rsm.onEvent(RacingStateMachine.Event.ABORT_HEAT_RECEIVED, this);
  }

  public void raceStarted() throws SerialPortException {
    if (gateOpen) {
      // If we get two "@"'s in a row, somehow, then we've effectively missed
      // a gate closure in between.
      rsm.onEvent(RacingStateMachine.Event.GATE_CLOSED, this);
    }

    gateOpen = true;
    rsm.onEvent(RacingStateMachine.Event.GATE_OPENED, this);
    invokeRaceStartedCallback();
  }

  @Override
  protected void raceFinished(Message.LaneResult[] results)
      throws SerialPortException {
    super.raceFinished(results);
    // Finishing the race will bring us to IDLE.
    // Now immediately handle as a gate closed event in anticipation of the
    // next race.
    gateOpen = false;
    rsm.onEvent(RacingStateMachine.Event.GATE_CLOSED, this);
  }

  // We can't directly interrogate, so we just return whatever state we've
  // heard from the timer.  This method shouldn't get called anyway, though,
  // because we're overriding poll(), below.
  @Override
  protected boolean interrogateGateIsClosed()
      throws NoResponseException, SerialPortException, LostConnectionException {
    return !gateOpen;
  }

  public int getNumberOfLanes() throws SerialPortException {
    // FastTrack, at least older versions, doesn't report actual number of lanes.
    return 0;
  }

  public void poll() throws SerialPortException, LostConnectionException {
    whileInState(rsm.state(this));
  }

public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState) {
    if (newState == RacingStateMachine.State.RESULTS_OVERDUE) {
      logOverdueResults();
    }
  }

  protected void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException {
    if (state == RacingStateMachine.State.RESULTS_OVERDUE) {
      rsm.onEvent(RacingStateMachine.Event.GATE_CLOSED, this);
      // This forces the state machine back to IDLE.
      rsm.onEvent(RacingStateMachine.Event.RESULTS_RECEIVED, this);
      portWrapper.logWriter().serialPortLogInternal(
          "No result from timer for the running race; giving up.");
    }
  }

}
