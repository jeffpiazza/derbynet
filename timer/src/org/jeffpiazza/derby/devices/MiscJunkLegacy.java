package org.jeffpiazza.derby.devices;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public class MiscJunkLegacy extends TimerDeviceCommon  {
  public MiscJunkLegacy(SerialPortWrapper portWrapper) {
    super(portWrapper, null);

    // Can't pass to super constructor because of the call to okToPoll()
    this.gateWatcher = new GateWatcher(portWrapper) {
      @Override
      protected boolean interrogateGateIsClosed()
          throws NoResponseException, SerialPortException,
                 LostConnectionException {
        if (!okToPoll()) {
          return getGateIsClosed(); // Reports Gate closed when in RACING and FINISHED states
        }
        portWrapper.write(POLL_GATE);
        long deadline = System.currentTimeMillis() + 500;
        String s;
        while ((s = portWrapper.next(deadline)) != null) {
          if (s.indexOf("O") >= 0) {
            return false;
          }

          if (s.indexOf(".") >= 0) {
            return true;
          }
        }
        throw new NoResponseException();
      }
    };

    // Once started, we expect a race result within 10 seconds; we allow an
    // extra second before considering the results overdue.
    rsm.setMaxRunningTimeLimit(11000);
  }

  private boolean okToPoll = true;

  public boolean okToPoll() {
    return okToPoll;
  }

  public void setOkToPoll(boolean v) {
    okToPoll = v;
  }

  public static String toHumanString() {
    return "PDT timer (https://www.dfgtec.com/pdt)";
  }

  private int numberOfLanes = 0;
  private ArrayList<Message.LaneResult> results;

  private static final String READ_VERSION = "V";
  private static final String READ_LANE_COUNT = "N";
  private static final String RESET = "R";
  private static final String MASK_LANE = "M";
  private static final String UNMASK_ALL_LANES = "U";
  private static final String POLL_GATE = "G";
  private static final String FORCE_END_OF_RACE = "F";
  private static final String START_SOLENOID = "S";

  @Override
  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE)) {
      return false;
    }
    // We just reset the timer, give it 2s to startup
    try {
      Thread.sleep(2000); // ms.
    } catch (Exception exc) {
    }
    portWrapper.write(READ_VERSION);
    long deadline = System.currentTimeMillis() + 500;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.indexOf("vert=") >= 0) {
        timerIdentifier = s;
        // Responds either "P" or "O"
        portWrapper.writeAndWaitForResponse(RESET, 500);

        String nl = portWrapper.writeAndWaitForResponse(READ_LANE_COUNT, 500);
        int nl_index = nl.indexOf("numl=");
        if (nl_index >= 0 && nl.length() > nl_index + 5) {
          this.numberOfLanes = nl.charAt(nl_index + 5) - '0';
          if (0 < numberOfLanes && numberOfLanes <= 9) {
            LogWriter.serial(this.numberOfLanes + " lane(s) reported.");
            setUp();
            return true;
          }
        }
      }
    }

    return false;
  }

  private static final Pattern resultLine
      = Pattern.compile("(\\d) - (\\d+\\.\\d+)");

  private void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
      public String apply(String line) throws SerialPortException {
        if (line.equals("B")) {
          // The timer announces a race start with "B" and moves to RACING state.
          // In RACING state it stops responding to gate state queries.  Continuing to
          // poll will lead to perceived connection timeouts. TIMER LED=purple
          setOkToPoll(false);
          onGateStateChange(false);
          return "";
        }
        if (line.equals("K")) {
          // The timer announces it is ready for the next race with "K" and moves to
          // READY state.  If the Arduino code does have GATE_RESET=1, the operator must
          // press a physical reset button on the timer to move to READY. TIMER LED=green
          setOkToPoll(true);
          return "";
        }
        Matcher m = resultLine.matcher(line);
        if (m.find()) {
          int lane = Integer.parseInt(m.group(1));
          LogWriter.serial("Detected result for lane " + lane + " = " + m.group(
              2));  // TODO
          if (results != null) {
            TimerDeviceUtils.addOneLaneResult(lane, m.group(2), -1, results);
            // Results are sent in lane order, so results are complete when
            // we see an entry for the last lane.
            if (lane == numberOfLanes) {
              raceFinished((Message.LaneResult[]) results.toArray(
                  new Message.LaneResult[results.size()]));
              results = null;
              // Timer state is FINISHED and results are displaying on the timer.
              // Having received results, we need to wait for the timer to be reset.
              // TIMER LED=red
              //setOkToPoll(true);
            }
          } else {
            LogWriter.serial("*** Unexpected lane result");
          }
          return "";
        } else {
          return line;
        }
      }
    });
  }

  private RemoteStartInterface remote_start = new RemoteStartInterface() {
    @Override
    public boolean hasRemoteStart() {
      return true;
    }

    @Override
    public void remoteStart() throws SerialPortException {
      portWrapper.writeAndDrainResponse(START_SOLENOID);
    }
  };

  @Override
  public RemoteStartInterface getRemoteStart() {
    return remote_start;
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return numberOfLanes;
  }

  @Override
  protected void maskLanes(int lanemask) throws SerialPortException {
    doMaskLanes(lanemask, UNMASK_ALL_LANES, 1, MASK_LANE, '1', 1);
  }

  @Override
  protected void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException {
    if (state == RacingStateMachine.State.RESULTS_OVERDUE) {
      // Upon entering RESULTS_OVERDUE state, we sent FORCE_END_OF_RACE; see
      // onTransition.
      if (portWrapper.millisSinceLastContact() > 1000) {
        // Track was RACING and we forced the end of race, we need to wait for READY
        // before we start polling.
        throw new LostConnectionException();
      } else if (rsm.millisInCurrentState() > 1000) {
        giveUpOnOverdueResults();
      }
    }
  }

  @Override
  public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState)
      throws SerialPortException {
    if (newState == RacingStateMachine.State.MARK) {
      results = new ArrayList<Message.LaneResult>();
    } else if (newState == RacingStateMachine.State.RESULTS_OVERDUE) {
      portWrapper.write(FORCE_END_OF_RACE);
      logOverdueResults();
    }
  }
}
