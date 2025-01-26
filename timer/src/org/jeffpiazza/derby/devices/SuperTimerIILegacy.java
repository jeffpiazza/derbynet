package org.jeffpiazza.derby.devices;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;

// This class supports the "Super Timer II" device
public class SuperTimerIILegacy extends TimerDeviceTypical {

  public SuperTimerIILegacy(TimerPortWrapper portWrapper) {
    super(portWrapper);

    // Once started, we expect a race result within 10 seconds
    rsm.setMaxRunningTimeLimit(10000);
  }

  public static String toHumanString() {
    return "Super Timer II";
  }

  // Invalid methods
  @Override
  protected synchronized boolean interrogateGateIsClosed()
      throws NoResponseException, SerialPortException, LostConnectionException {
        return true;
  }
  // END Invalid methods

  private int laneCount = 6;

  // These get initialized upon the start of each new race.
  private int nresults = 0;
  private ArrayList<Message.LaneResult> results;

  // R => 'RESET' crlf 'READY <n> LANES' crlf
  private static final String RESET_COMMAND = "3@5A";

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
    setUp();
    return true;
  }

  private static final Pattern singleLanePattern = Pattern.compile(
      "..#([1-6])..\\s+(\\d+)(\\s.*)");
  private static final Pattern raceOverPattern = Pattern.compile("..!..");

  protected void setUp() {
    portWrapper.registerDetector(new TimerPortWrapper.Detector() {
      @Override
      public String apply(String line) throws SerialPortException {
          Matcher m = singleLanePattern.matcher(line);
          if (m.find()) {
            int lane = Integer.parseInt(m.group(1));
            String time = m.group(2);
            if (results != null) {
              int decimalPosition = 4; // Decimal location
              StringBuilder sb = new StringBuilder(time);
              sb.insert(time.length() - decimalPosition, '.');
              TimerDeviceUtils.addOneLaneResult(lane, sb.toString(), -1, results);
            }
            ++nresults;
            return "";
          } else if (raceOverPattern.matcher(line).find()) {
          // invokeRaceFinishedCallback(roundid, heat, result.toArray());
          roundid = heat = 0;
          return "";
        }
          return line;
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
    portWrapper.writeAndDrainResponse("3" + (char)lanemask+1 + "5A");
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
