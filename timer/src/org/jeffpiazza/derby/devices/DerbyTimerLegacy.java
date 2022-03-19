package org.jeffpiazza.derby.devices;

import java.util.ArrayList;
import jssc.*;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.jeffpiazza.derby.LogWriter;

// This class supports the "Derby Timer" device, http://derbytimer.com
public class DerbyTimerLegacy extends TimerDeviceTypical {

  public DerbyTimerLegacy(SerialPortWrapper portWrapper) {
    super(portWrapper);

    // Once started, we expect a race result within 10 seconds
    rsm.setMaxRunningTimeLimit(10000);
  }

  public static String toHumanString() {
    return "Derby Timer";
  }

  private int laneCount = 0;

  // These get initialized upon the start of each new race.
  private int nresults = 0;
  private ArrayList<Message.LaneResult> results;

  // R => 'RESET' crlf 'READY <n> LANES' crlf
  private static final String RESET_COMMAND = "R";
  private static final String LANE_MASK = "M";
  // C => 'OK' crlf
  private static final String CLEAR_LANE_MASK = "C";

  // Response to a "G" is either "U" (up, or closed) or "D" (down, or open)
  private static final String READ_START_SWITCH = "G";

  private static final String FORCE_RACE_RESULTS = "F";

  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE,
                                   /* rts */ true,
                                   /* dtr */ true)) {
      return false;
    }

    portWrapper.write(RESET_COMMAND);
    long deadline = System.currentTimeMillis() + 2000;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.equals("RESET")) {
        s = portWrapper.next(deadline);
        if (s != null) {
          Matcher m = readyNLanesPattern.matcher(s);
          if (m.find()) {
            laneCount = Integer.parseInt(m.group(1));
            timerIdentifier = s;
          }

          setUp();
          return true;
        }
      }
    }

    return false;
  }

  private static final Pattern readyNLanesPattern = Pattern.compile(
      "^READY\\s*(\\d+)\\s+LANES");
  private static final Pattern singleLanePattern = Pattern.compile(
      "^\\s*(\\d)\\s+(\\d\\.\\d+)(\\s.*|)");

  protected void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
      @Override
      public String apply(String line) throws SerialPortException {
        if (line.equals("RACE")) {
          if (getGateIsClosed()) {
            setGateIsClosed(false);
            rsm.onEvent(RacingStateMachine.Event.GATE_OPENED);
          }
          return "";
        } else if (line.equals("FINISH")) {
          if (results != null) {
            raceFinished((Message.LaneResult[]) results.toArray(
                new Message.LaneResult[results.size()]));
            results = null;
            nresults = 0;
          }
          return "";
        } else {
          Matcher m = readyNLanesPattern.matcher(line);
          if (m.find()) {
            int nlanes = Integer.parseInt(m.group(1));
            // If any lanes have been masked, not sure what READY n LANES
            // will report, so only update a larger laneCount.
            if (nlanes > laneCount) {
              laneCount = nlanes;
            }
            if (!getGateIsClosed()) {
              setGateIsClosed(true);
              rsm.onEvent(RacingStateMachine.Event.GATE_CLOSED);
            }
            return "";
          }
          m = singleLanePattern.matcher(line);
          if (m.find()) {
            int lane = Integer.parseInt(m.group(1));
            String time = m.group(2);
            if (results != null) {
              TimerDeviceUtils.addOneLaneResult(lane, time, -1, results);
            }
            ++nresults;
            return "";
          }
          return line;
        }
      }
    });
  }

  // Timer reports overdue results as 0.0000, but we need to report them
  // as 9.9999.
  @Override
  protected void raceFinished(Message.LaneResult[] results)
      throws SerialPortException {
    super.raceFinished(TimerDeviceUtils.zeroesToNines(results));
  }

  protected void maskLanes(int lanemask) throws SerialPortException {
    portWrapper.writeAndDrainResponse(CLEAR_LANE_MASK);

    for (int lane = 0; lane < laneCount; ++lane) {
      if ((lanemask & (1 << lane)) == 0) {
        // Response is "MASKING LANE <n>"
        portWrapper.writeAndDrainResponse(
            LANE_MASK + (char) ('1' + lane), 1, 500);
      }
    }
  }

  // Interrogates the starting gate's state.
  @Override
  protected synchronized boolean interrogateGateIsClosed()
      throws NoResponseException, SerialPortException, LostConnectionException {
    portWrapper.write(READ_START_SWITCH);
    long deadline = System.currentTimeMillis() + 1000;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.trim().equals("U")) {
        return true;
      } else if (s.trim().equals("D")) {
        return false;
      } else {
        LogWriter.serial("Unrecognized response: '" + s + "'");
      }
    }

    throw new NoResponseException();
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return laneCount;
  }

  @Override
  public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState)
      throws SerialPortException {
    if (newState == RacingStateMachine.State.MARK) {
      nresults = 0;
      results = new ArrayList<Message.LaneResult>();
    } else if (newState == RacingStateMachine.State.RESULTS_OVERDUE) {
      // Force results upon entering RESULTS_OVERDUE.  After another second
      // (in whileInState), give up and revert to idle.
      portWrapper.write(FORCE_RACE_RESULTS);
      logOverdueResults();
    }
  }

  protected void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException {
    if (state == RacingStateMachine.State.RESULTS_OVERDUE) {
      // FORCE_RACE_RESULTS was sent upon entering RESULTS_OVERDUE; see above.
      if (rsm.millisInCurrentState() > 1000) {
        giveUpOnOverdueResults();
      }
    }
  }
}
