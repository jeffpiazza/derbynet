package org.jeffpiazza.derby.devices;

import jssc.*;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

import java.util.regex.Matcher;

public class FastTrackDevice extends TimerDeviceTypical {

  public FastTrackDevice(SerialPortWrapper portWrapper) {
    super(portWrapper);

    // Once started, we expect a race result within 10 seconds; we allow an
    // extra second before considering the results overdue.
    rsm.setMaxRunningTimeLimit(11000);
  }

  public static String toHumanString() {
    return "FastTrack timer";
  }

  public static final int MAX_LANES = 6;

  private static final String PULSE_LASER_BIT = "LG";
  // LG actually starts the timer, but after a brief pause
  private static final String RESET_LASER_GATE = "LR";
  // LR appears to be the real "reset" command for the timer
  private static final String LANE_MASK = "M"; // + A to mask out lane 1, B lane 2, etc.
  private static final String CLEAR_LANE_MASK = LANE_MASK + "G";
  private static final String OLD_FORMAT = "N0"; //A=3.001! B=3.002 C=3.003 D=3.004 E=3.005 F=3.006 <LF> <CR>
  private static final String NEW_FORMAT = "N1"; //A=3.001! B=3.002" C=3.003# D=3.004$ E=3.005% F=3.006& <CR> <LF>
  private static final String ENHANCED_FORMAT = "N2";
  // N2 => 5-digit time and start switch open/cloed status, 2012 or newer timers only
  // private static final String COUNT_DOWN_TIMER = "PC"; // e.g., PC01 to count down one minute
  private static final String FORCE_RESULTS = "RA";
  // RA doesn't report anything unless at least one car has crossed the line
  // But it will stop the timer...
  private static final String RESET_ELIMINATOR_MODE = "RE";
  private static final String READ_START_SWITCH = "RG";
  private static final String REVERSE_LANES = "RL";  // + 0-6, number of lanes on track
  private static final String READ_MODE = "RM";
  private static final String READ_SERIAL_NUMBER = "RS";
  private static final String READ_VERSION = "RV";
  // private static final String FORCE_PRINT = "RX";  // requires "Force Print" option
  // RX resets the timer, but then seems to make it unresponsive

  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE,
                                   /* rts */ false,
                                   /* dtr */ false)) {
      return false;
    }

    portWrapper.write(READ_VERSION);

    // We're looking for a response that matches these:
    // Copyright (c) Micro Wizard 2002-2005
    // K3 Version 1.05A  Serial Number 15985
    long deadline = System.currentTimeMillis() + 2000;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.indexOf("Micro Wizard") >= 0) {
        portWrapper.logWriter().serialPortLogInternal("* Micro Wizard detected");
        s = portWrapper.next(deadline);
        if (s.startsWith("K")) {
          portWrapper.logWriter().serialPortLogInternal(
              "* K timer string detected");
          setUp();
          return true;
        }
      }
    }

    return false;
  }

  protected void setUp() {
    // TODO Can we take control of the timer in a way to show that it's been recognized?
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
  }

  public void prepareHeat(int roundid, int heat, int lanemask) throws
      SerialPortException {
    prepare(roundid, heat);
    portWrapper.write(CLEAR_LANE_MASK);
    // The CLEAR_LANE_MASK causes an "AC" response, but without a cr/lf to mark
    // a complete response.
    StringBuilder sb = new StringBuilder("Heat prepared: ");
    for (int lane = 0; lane < MAX_LANES; ++lane) {
      if ((lanemask & (1 << lane)) != 0) {
        sb.append(lane + 1);
      } else {
        sb.append("-");
        // A LANE_MASK command echoes the command (first response) and then
        // sends a "* <cr> <lf>" (second response).
        portWrapper.writeAndDrainResponse(
            LANE_MASK + (char) ('A' + lane), 2, 2000);
      }
    }
    portWrapper.logWriter().serialPortLogInternal(sb.toString());
    rsm.onEvent(RacingStateMachine.Event.PREPARE_HEAT_RECEIVED, this);
  }

  // Interrogates the starting gate's state.  CAUTION: polling while a
  // race is running may cause the race results, sent asynchronously, to
  // be mixed with the RG response, making them unintelligible.
  @Override
  protected boolean interrogateGateIsClosed()
      throws NoResponseException, SerialPortException, LostConnectionException {
    portWrapper.write(READ_START_SWITCH);
    long deadline = System.currentTimeMillis() + 1000;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.startsWith(READ_START_SWITCH)) {
        return (s.charAt(2) == '1');
      }
    }
    throw new NoResponseException();
  }

  public int getNumberOfLanes() throws SerialPortException {
    // FastTrack, at least older versions, doesn't report actual number of lanes.
    return 0;
  }

  @Override
  public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState) {
    if (newState == RacingStateMachine.State.RESULTS_OVERDUE) {
      logOverdueResults();
    }
  }

  protected void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException {
    if (state == RacingStateMachine.State.RESULTS_OVERDUE) {
      // A reasonably common scenario is this: if the gate opens accidentally
      // after the PREPARE_HEAT, the timer starts but there are no cars to
      // trigger a result.
      //
      // updateGateIsClosed() may throw a LostConnectionException if the
      // timer has become unresponsive; otherwise, we'll deal with an
      // unexpected gate closure (which has no real effect).
      if (updateGateIsClosed()) {
        // It can certainly happen that the gate gets closed while the race
        // is running.
        rsm.onEvent(RacingStateMachine.Event.GATE_CLOSED, this);
      }
      // This forces the state machine back to IDLE.
      rsm.onEvent(RacingStateMachine.Event.RESULTS_RECEIVED, this);
      // TODO invokeMalfunctionCallback(false, "No result received from last heat.");
      // We'd like to alert the operator to intervene manually, but
      // as currently implemented, a malfunction(false) message would require
      // unplugging/replugging the timer to reset: too invasive.
      portWrapper.logWriter().serialPortLogInternal(
          "No result from timer for the running race; giving up.");
    } else if (state == RacingStateMachine.State.MARK) {
      // prepareHeat() called; waiting for gate to close.
      // Detecting "gate closed" and getting to SET depends on
      // resetting the laser if a laser start gate; we do that continuously
      // until detecting the gate "closed" (laser hits sensor).
      portWrapper.writeAndDrainResponse(RESET_LASER_GATE, 2, 2000);
      checkConnection();
    }
  }
}
