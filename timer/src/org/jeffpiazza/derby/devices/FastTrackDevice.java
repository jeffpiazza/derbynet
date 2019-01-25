package org.jeffpiazza.derby.devices;

import jssc.*;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

import java.util.regex.Matcher;

public class FastTrackDevice extends TimerDeviceCommon {
  public FastTrackDevice(SerialPortWrapper portWrapper) {
    super(portWrapper, null);
    gateWatcher = new GateWatcher(portWrapper) {
        // Interrogates the starting gate's state.  CAUTION: polling while a
        // race is running may cause the race results, sent asynchronously, to
        // be mixed with the RG response, making them unintelligible.
        @Override
        protected boolean interrogateGateIsClosed()
            throws NoResponseException, SerialPortException,
                   LostConnectionException {
          portWrapper.write(READ_START_SWITCH);
          long deadline = System.currentTimeMillis() + 1000;
          String s;
          while ((s = portWrapper.next(deadline)) != null) {
            if (s.startsWith(READ_START_SWITCH)) {
              if (s.length() >= 3) {
                return s.charAt(2) == '1';
              }
              // K1 timer seems to respond "RG" followed by separate "X"
              // response, but it doesn't otherwise appear to show the gate
              // state.
              // Per Stuart Ferguson (18 Jan 2019):
              // The "X" indicates that the option is disabled. We will need
              // the serial number of your timer to generate a code that will
              // enable this feature.  You can then purchase the unlock code for
              // $20 at http://microwizard.com/orderform.php
              s = portWrapper.next(deadline);
              if (s != null) {
                if (s.equals("X")) {
                  setGateStateNotKnowable();
                }
              }
            }
          }
          throw new NoResponseException();
        }
      };

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
  // N2 => 5-digit time and start switch open/closed status, 2012 or newer timers only
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
        timerIdentifier = s;
        s = portWrapper.next(deadline);
        if (s.startsWith("K")) {
          // Clean up the timer state and capture some details into the log
          portWrapper.writeAndDrainResponse(RESET_ELIMINATOR_MODE, 2, 1000);
          portWrapper.writeAndDrainResponse(NEW_FORMAT, 2, 1000);
          // This "RM" command seems to silence the K1 timer.
          // TODO portWrapper.writeAndDrainResponse(READ_MODE);
          setUp();
          return true;
        }
      }
    }

    return false;
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
  }

  @Override
  public int getSafeNumberOfLanes() {
    return MAX_LANES;
  }

  @Override
  protected void maskLanes(int lanemask) throws SerialPortException {
    // The CLEAR_LANE_MASK causes an "AC" response, but without a cr/lf to mark
    // a complete response.
    doMaskLanes(lanemask, CLEAR_LANE_MASK, 0, LANE_MASK, 'A', 2);
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
      giveUpOnOverdueResults();
    } else if (state == RacingStateMachine.State.MARK && gateWatcher != null) {
      // prepareHeat() called; waiting for gate to close.
      // Detecting "gate closed" and getting to SET depends on
      // resetting the laser if a laser start gate; we do that continuously
      // until detecting the gate "closed" (laser hits sensor).
      //
      // If the gate state option is disabled, then we don't want to be
      // continuously resetting the laser gate, because we could receive results
      // at any instant, and they'd be disrupted by the reset dialog.
      portWrapper.writeAndDrainResponse(RESET_LASER_GATE, 2, 2000);
      checkConnection();
    }
  }
}
