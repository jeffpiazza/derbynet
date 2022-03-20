package org.jeffpiazza.derby.devices;

import java.util.regex.Matcher;
import jssc.*;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public class SmartLineLegacy extends TimerDeviceCommon {
  private int numberOfLanes;  // Detected at probe time

  private static final String READ_DECIMAL_PLACES = "od\r";
  private static final String SET_DECIMAL_PLACES = "od";  // 3,4,5
  private static final String READ_LANE_CHARACTER = "ol\r";
  private static final String SET_LANE_CHARACTER_A = "ol0\r"; // report lane 1 as "A"
  // private static final String READ_LANE_MASK = "om\r";
  private static final String RESET_LANE_MASK = "om0\r";
  private static final String MASK_LANE = "om";  // + lane, e.g., om3
  private static final String READ_LANE_COUNT = "on\r";
  // private static final String SET_LANE_COUNT = "on"; // + count, eg on4
  private static final String READ_PLACE_CHARACTER = "op\r";
  private static final String SET_PLACE_CHARACTER_BANG = "op3\r";
  private static final String READ_AUTO_RESET = "or\r";
  private static final String RESET_REVERSE_LANES = "ov0\r";
  private static final String RESET_DTX000_MODE = "ox0\r";

  private static final String RESET = "r\r";
  private static final String FORCE_END_OF_RACE = "ra\r";
  private static final String RETURN_RESULTS_WHEN_RACE_ENDS = "rg\r";
  private static final String READ_FINISH_LINE = "rl\r";  // bit mask, 0=inactive
  // private static final String RETURN_PREVIOUS = "rp\r";
  private static final String READ_RESET_SWITCH = "rr\r";
  private static final String READ_START_SWITCH = "rs\r";
  private static final String READ_VERSION = "v\r";

  public SmartLineLegacy(SerialPortWrapper portWrapper) {
    super(portWrapper, new GateWatcher(portWrapper) {
        @Override
        protected boolean interrogateGateIsClosed()
            throws NoResponseException, SerialPortException,
                   LostConnectionException {
          portWrapper.write(READ_START_SWITCH);
          long deadline = System.currentTimeMillis() + 500;
          String s;
          while ((s = portWrapper.next(deadline)) != null) {
            if (s.equals("1")) {
              return false;
            } else if (s.equals("0")) {
              return true;
            }
          }
          throw new NoResponseException();
        }
      });

    // Once started, we expect a race result within 10 seconds; we allow an
    // extra second before considering the results overdue.
    rsm.setMaxRunningTimeLimit(11000);
  }

  public static String toHumanString() {
    return "\"The Champ\" (SmartLine/BestTrack)";
  }

  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE)) {
      return false;
    }

    // Just forcing a new line, don't care about response.
    portWrapper.writeAndDrainResponse("\r");

    portWrapper.write(READ_VERSION);

    long deadline = System.currentTimeMillis() + 2000;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      // eTekGadget SmartLine Timer v20.06 (B0007)
      if (s.indexOf("eTekGadget SmartLine Timer") >= 0) {
        timerIdentifier = s;

        portWrapper.write(RESET);

        String nl = portWrapper.writeAndWaitForResponse(READ_LANE_COUNT, 500);
        if (nl != null && nl.length() > 0 &&
            '0' < nl.charAt(0) && nl.charAt(0) <= '9') {
          this.numberOfLanes = nl.charAt(0) - '0';
          LogWriter.serial(this.numberOfLanes + " lane(s) reported.");
        }

        // TODO: Does this just need to be configured to
        // eliminate having to do manually?
        LogWriter.serial("AUTO_RESET = "
            + portWrapper.writeAndWaitForResponse(READ_AUTO_RESET, 500)
        );
        LogWriter.serial("LANE_CHARACTER = "
            + portWrapper.writeAndWaitForResponse(READ_LANE_CHARACTER, 500)
        );
        LogWriter.serial("DECIMAL_PLACES = "
            + portWrapper.writeAndWaitForResponse(READ_DECIMAL_PLACES, 500)
        );
        LogWriter.serial("PLACE_CHARACTER = "
            + portWrapper.writeAndWaitForResponse(READ_PLACE_CHARACTER, 500)
        );
        LogWriter.serial("START_SWITCH = "
            + portWrapper.writeAndWaitForResponse(READ_START_SWITCH, 500)
        );

        portWrapper.writeAndDrainResponse(SET_LANE_CHARACTER_A, 1, 500);
        portWrapper.writeAndDrainResponse(SET_PLACE_CHARACTER_BANG, 1, 500);

        setUp();
        return true;
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
                                                getSafeNumberOfLanes());
          raceFinished(results);
          return line.substring(0, m.start()) + line.substring(m.end());
        } else {
          return line;
        }
      }
    });
  }

  public int getNumberOfLanes() throws SerialPortException {
    return numberOfLanes;
  }

  private static final int MAX_LANES = 6;

  public int getSafeNumberOfLanes() {
    return numberOfLanes == 0 ? MAX_LANES : numberOfLanes;
  }

  protected void maskLanes(int lanemask) throws SerialPortException {
    // Need "\r" at the end of MASK_LANE command; that's what keeps us from
    // using doMaskLanes.

    // These don't give responses, so no need to wait for any.
    portWrapper.write(RESET_LANE_MASK);

    for (int lane = 0; lane < getSafeNumberOfLanes(); ++lane) {
      if ((lanemask & (1 << lane)) == 0) {
        // 100ms. sleep between mask commands
        try { Thread.sleep(100); } catch (InterruptedException ex) { }
        portWrapper.write(MASK_LANE + (char) ('1' + lane) + "\r");
      }
    }
  }

  public void abortHeat() throws SerialPortException {
    rsm.onEvent(RacingStateMachine.Event.ABORT_HEAT_RECEIVED);
  }

  @Override
  public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState)
      throws SerialPortException {
    if (newState == RacingStateMachine.State.RUNNING) {
      // TODO Seems a little precarious to wait until RUNNING to send this.
      // Can this be sent during the SET state?
      portWrapper.write(RETURN_RESULTS_WHEN_RACE_ENDS);
    } else if (newState == RacingStateMachine.State.RESULTS_OVERDUE) {
      portWrapper.write(FORCE_END_OF_RACE);
      logOverdueResults();
    }
  }

  @Override
  protected void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException {
    if (state == RacingStateMachine.State.RESULTS_OVERDUE) {
      // Upon entering RESULTS_OVERDUE state, we sent FORCE_END_OF_RACE; see
      // onTransition.  Because we stopped polling while the race was running,
      // a checkConnection is likely to throw LostConnectionException, so we
      // avoid doing a checkConnection until the timer's had a chance to
      // respond to the FORCE.
      if (rsm.millisInCurrentState() > 1000) {
        checkConnection();
        giveUpOnOverdueResults();
      }
    }
  }
}
