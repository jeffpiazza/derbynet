package org.jeffpiazza.derby.devices;

import jssc.*;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.SerialPortWrapper;
import org.jeffpiazza.derby.Timestamp;

import java.util.regex.Matcher;

public class FastTrackDevice extends TimerDeviceBase implements TimerDevice {
  // States for the timer:
  private int state = IDLE;
  private void setState(int newState) {
    state = newState;
    portWrapper.logWriter().serialPortLogInternal("setState(" + (state == IDLE ? "IDLE" :
                                                                 state == MARK ? "MARK" :
                                                                 state == SET  ? "SET" :
                                                                 state == RUNNING ? "RUNNING" :
                                                                 "UNKNOWN")
                                                  + ")");
  }

  // Nothing going on; waiting for a prepareHeat call.
  private static final int IDLE = 0;

  // prepareHeat has been called, and a lane mask set.  Waiting for
  // the gate to be closed.
  private static final int MARK = 1;

  // Gate is closed, waiting for the race to start
  private static final int SET = 2;

  // Gate has opened, starting the race.
  private static final int RUNNING = 3;

  public FastTrackDevice(SerialPortWrapper portWrapper) {
    super(portWrapper);
  }

  protected void raceFinished(Message.LaneResult[] results) {
    setState(IDLE);
    super.raceFinished(results);
  }

  public static final int MAX_LANES = 6;

  private static final String PULSE_LASER_BIT = "LG";
  // LG actually starts the timer, but after a brief pause
  private static final String RESET_LASER_GATE = "LR";
  // LR appears to be the real "reset" command for the timer
  private static final String LANE_MASK = "M"; // + A to mask out lane 1, B lane 2, etc.
  private static final String CLEAR_LANE_MASK = LANE_MASK + "G";
  private static final String OLD_FORMAT = "N0"; //A=3.001! B=3.002 C=3.003 D=3.004 E=3.005 F=3.006 <LF> <CR>
  private static final String NEW_FORMAT = "N1"; //A=3.001! B=3.002” C=3.003# D=3.004$ E=3.005% F=3.006& <CR> <LF>
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

  // Keeps track of last known state of the gate
  private boolean gateIsClosed;
  // Interrogates the starting gate's state.  CAUTION: polling while a
  // race is running may cause the race results, sent asynchronously, to
  // be mixed with the RG response, making them unintelligible.
  private boolean isGateClosed() throws SerialPortException {
    portWrapper.write(READ_START_SWITCH);

    long deadline = System.currentTimeMillis() + 1000;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.startsWith(READ_START_SWITCH)) {
        gateIsClosed = (s.charAt(2) == '1');
        return gateIsClosed;
      }
    }

    checkConnection();

    // Don't know, assume unchanged
    portWrapper.logWriter().serialPortLogInternal("*** Unable to determine starting gate state");
    System.err.println(Timestamp.string() + ": Unable to read starting gate state");
    return gateIsClosed;
  }

  public boolean probe() throws SerialPortException {
    if (!portWrapper.port().setParams(SerialPort.BAUDRATE_9600, SerialPort.DATABITS_8,
                                      SerialPort.STOPBITS_1, SerialPort.PARITY_NONE,
                                      /* rts */ false, /* dtr */ false)) {
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
          portWrapper.logWriter().serialPortLogInternal("* K timer string detected");
          setUp();
          return true;
        }
      }
    }

    return false;
  }

  protected void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
        public String apply(String line) {
          Matcher m = TimerDeviceUtils.matchedCommonRaceResults(line);
          if (m != null) {
            Message.LaneResult[] results =
                TimerDeviceUtils.extractResults(line, m.start(), m.end(), MAX_LANES);
            raceFinished(results);
            return line.substring(0, m.start()) + line.substring(m.end());
          } else {
            return line;
          }
        }
      });
  }

  public int getNumberOfLanes() throws SerialPortException {
    // FastTrack, at least older versions, doesn't report actual number of lanes.
    return 0;
  }
  
  public void prepareHeat(int lanemask) throws SerialPortException {
    // TODO: If state != IDLE then what?

    portWrapper.write(CLEAR_LANE_MASK);
    // The CLEAR_LANE_MASK causes an "AC" response, but without a cr/lf to mark
    // a complete response.

    StringBuffer sb = new StringBuffer("Heat prepared: ");
    for (int lane = 0; lane < MAX_LANES; ++lane) {
      if ((lanemask & (1 << lane)) != 0) {
        sb.append(lane + 1);
      } else {
        sb.append("-");
        // A LANE_MASK command echoes the command (first response) and then
        // sends a "* <cr> <lf>" (second response).
        portWrapper.writeAndDrainResponse(LANE_MASK + (char)('A' + lane), 2, 2000);
      }
    }
    portWrapper.logWriter().serialPortLogInternal(sb.toString());

    setState(MARK);
  }

  public void abortHeat() throws SerialPortException {
    setState(IDLE);
  }
  
  public void poll() throws SerialPortException {
    switch (state) {
    case IDLE:
      if (portWrapper.millisSinceLastCommand() > 500) {
        // This call to isGateClosed is just to confirm that the connection is
        // functioning; we only need to check it occasionally.
        isGateClosed();
      }
      return;
    case MARK:
      // prepareHeat() called; waiting for gate to close
      portWrapper.writeAndDrainResponse(RESET_LASER_GATE, 2, 2000);
      checkConnection();

      if (isGateClosed()) {
        setState(SET);
        startGateChange(/* isOpen */ false);
      }
      return;
    case SET:
      // prepareHeat() called, gate closed, waiting for gate to open
      if (!isGateClosed()) {
        setState(RUNNING);
        raceStarted();
        startGateChange(/* isOpen */ true);
      }
      return;
    case RUNNING:
      // Don't disturb a running timer!
      return;
    }
  }
}
