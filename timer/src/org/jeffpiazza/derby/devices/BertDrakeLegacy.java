package org.jeffpiazza.derby.devices;

import jssc.*;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.jeffpiazza.derby.LogWriter;

// This class supports PICmicro-based DIY timer designed by Bert Drake,
// described at http://drakedev.com/pinewood/.  The "Race Timer Software
// Interface Protocol" is described at
// http://drakedev.com/pinewood/protocol.html
public class BertDrakeLegacy extends TimerDeviceCommon {
  private TimerResult result = null;
  private int nresults = 0;

  public BertDrakeLegacy(SerialPortWrapper portWrapper) {
    super(portWrapper, null);
    gateWatcher = new GateWatcher(portWrapper) {
      // Interrogates the starting gate's state.  CAUTION: polling while a
      // race is running may cause the race results, sent asynchronously, to
      // be mixed with the RG response, making them unintelligible.
      @Override
      protected boolean interrogateGateIsClosed()
          throws NoResponseException, SerialPortException,
                 LostConnectionException {
        portWrapper.write(CHECK_START_GATE_COMMAND);
        long deadline = System.currentTimeMillis() + 1000;
        String s;
        while ((s = portWrapper.next(deadline)) != null) {
          if (s.equals(GATE_IS_CLOSED)) {
            return true;
          }
          if (s.equals(GATE_IS_OPEN)) {
            return false;
          }
        }
        throw new NoResponseException();
      }
    };
    // The timer itself has a settable timeout value, so no need to try to
    // enforce one from this side.
  }

  public static String toHumanString() {
    return "Bert Drake timer";
  }

  private static final String RESET_COMMAND = "R";
  private static final String RETRIEVE_TIMES_COMMAND = "T";
  private static final String FORCE_END_COMMAND = "F";
  private static final String CHECK_START_GATE_COMMAND = "C";
  private static final String START_COMMAND = "S";

  private static final String VERSION_COMMAND = "V";
  private static final String DEBUG_MODE_COMMAND = "D";

  private static final String GATE_IS_CLOSED = "Gc";
  private static final String GATE_IS_OPEN = "Go";

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
    portWrapper.write(VERSION_COMMAND);
    // Pinewood Timer v1.26
    // by Bert Drake
    long deadline = System.currentTimeMillis() + 500;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.contains("Bert Drake")) {
        setUp();
        return true;
      }
    }

    return false;
  }

  private static final Pattern singleLanePattern = Pattern.compile(
      "^\\s*(\\d)\\s+(\\d\\.\\d+)(\\s.*|)");

  protected void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
      @Override
      public String apply(String line) throws SerialPortException {
        if (line.equals("B")) {
          rsm.onEvent(RacingStateMachine.Event.GATE_OPENED);
          return "";
        } else if (line.equals("E")) {
          return "";
        } else {
          Matcher m = singleLanePattern.matcher(line);
          if (m.find()) {
            int lane = Integer.parseInt(m.group(1));
            String time = m.group(2);
            if (rsm.state() != RacingStateMachine.State.RUNNING) {
              // We only accept results when the race is running, as a guard
              // against late results from one heat being applied after
              // receiving a heat-ready for the next heat.
              LogWriter.serial("Ignoring lane result because race isn't running");
            } else if (result != null) {
              ++nresults;
              result.setLane(lane, time, nresults);
              if (result.isFilled()) {
                // raceFinished may result in a new value for result member, so
                // clean up this one before the call
                Message.LaneResult[] resultArray = result.toArray();
                result = null;
                raceFinished(resultArray);
              }
            }
            return "";
          }
          return line;
        }
      }
    });
  }

  @Override
  public void abortHeat() throws SerialPortException {
    portWrapper.write(RESET_COMMAND);
    super.abortHeat();
  }

  // Timer reports overdue results as 0.0000, but we need to report them
  // as 9.9999.
  @Override
  protected void raceFinished(Message.LaneResult[] results)
      throws SerialPortException {
    super.raceFinished(TimerDeviceUtils.zeroesToNines(results));
  }

  protected void maskLanes(int lanemask) throws SerialPortException {
    result = new TimerResult(lanemask);
    nresults = 0;
    // The timer doesn't support lane masking; we'll call the race when
    // we get results from each of the expected lanes.
    portWrapper.write(RESET_COMMAND);
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return 0;
  }

  @Override
  public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState)
      throws SerialPortException {
    if (newState == RacingStateMachine.State.RESULTS_OVERDUE) {
      // Force results upon entering RESULTS_OVERDUE.  After another second
      // (in whileInState), give up and revert to idle.
      portWrapper.write(FORCE_END_COMMAND);
      portWrapper.write(RETRIEVE_TIMES_COMMAND);
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
