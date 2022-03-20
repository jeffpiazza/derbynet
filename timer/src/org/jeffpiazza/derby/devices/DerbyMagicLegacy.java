package org.jeffpiazza.derby.devices;

import jssc.*;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;

// TODO: "R" responds with "Ready", which maybe means this timer CAN be identified.
// More importantly, "V" responds with "Derby Magic v3.00" or some such.
// http://www.derbymagic.com/files/Timer.pdf
// http://www.derbymagic.com/files/GPRM.pdf
public class DerbyMagicLegacy extends TimerDeviceCommon {
  private TimerResult result = null;
  private long timeOfFirstResult = 0;

  public DerbyMagicLegacy(SerialPortWrapper portWrapper) {
    // No GateWatcher, because there's no way to poll this timer
    super(portWrapper, null, /* gate is knowable */ false);

    // Once started, we expect a race result within 10 seconds; we allow an
    // extra second before considering the results overdue.
    rsm.setMaxRunningTimeLimit(11000);
  }

  public static String toHumanString() {
    return "Derby Magic timer";
  }

  private static final int MAX_LANES = 8;

  private static final String READ_VERSION = "V";
  private static final String TIMER_HAS_STARTED = "B";

  private static final String TIMER_RESET = "R";
  private static final String FORCE_DATA_SEND = "F";

  private static final String TRIGGER_START_SOLENOID = "S";

  @Override
  public boolean canBeIdentified() {
    return true;
  }

  public boolean probe() throws SerialPortException {
    for (int baudrate : new int[]{9600, 19200}) {
      if (!portWrapper.setPortParams(baudrate,
                                     SerialPort.DATABITS_8,
                                     SerialPort.STOPBITS_1,
                                     SerialPort.PARITY_NONE)) {
        continue;
      }

      portWrapper.write(READ_VERSION);

      long deadline = System.currentTimeMillis() + 1000;
      String s;
      while ((s = portWrapper.next(deadline)) != null) {
        if (s.indexOf("Derby Magic") >= 0) {
          timerIdentifier = s;
          portWrapper.writeAndDrainResponse(TIMER_RESET, 1, 200);
          setUp();
          return true;
        }
      }
    }
    return false;
  }

  protected boolean probeAtSpeed(int baudrate) throws SerialPortException {
    if (!portWrapper.setPortParams(baudrate,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE)) {
      return false;
    }

    portWrapper.writeAndDrainResponse(TIMER_RESET, 1, 200);
    timerIdentifier = portWrapper.writeAndWaitForResponse(READ_VERSION);

    setUp();
    return true;
  }

  // Timer sends individual lane results as each car crosses the finish line.
  // Each result is <lane>=<time><place>, where <lane> is the lane number,
  // <time> is time (0.0000-9.9999), and <place> is a punctuation charactter,
  // !=1st, "=2nd, #=3rd, $=4th, %=5th, &=6th, *=7th, @=8th
  // (even though @ doesn't follow in ASCII order)
  private static final Pattern singleLanePattern = Pattern.compile(
      "([1-8])=(\\d\\.\\d+)([!-/:-@]) *");

  protected void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
      public String apply(String line) throws SerialPortException {
        if (line.equals(TIMER_HAS_STARTED)) {
          LogWriter.serial("Detected gate opening");
          // This will be an unexpected state change, if it ever happens
          onGateStateChange(false);
          return "";
        }
        return line;
      }
    });
    portWrapper.registerEarlyDetector(new SerialPortWrapper.Detector() {
      @Override
      public String apply(String line) throws SerialPortException {
        Matcher m = singleLanePattern.matcher(line);
        while (m.find()) {
          LogWriter.serial("    Early detector match for (" + m.group() + ")");
          int lane = m.group(1).charAt(0) - '1' + 1;
          String time = m.group(2);
          int place = 0;
          if (m.group(3).length() > 0) {
            place = m.group(3).charAt(0) - '!' + 1;
          }

          if (result == null) {
            LogWriter.serial("*    No results were expected now.");
          } else {
            result.setLane(lane, time, place);
            if (timeOfFirstResult == 0) {
              timeOfFirstResult = System.currentTimeMillis();
            }
          }

          line = line.substring(0, m.start()) + line.substring(m.end());
          m = singleLanePattern.matcher(line);
        }

        if (result != null && result.isFilled()) {
          raceFinished();
        }

        return line;
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
      portWrapper.write(TRIGGER_START_SOLENOID);
    }
  };

  @Override
  public RemoteStartInterface getRemoteStart() {
    return remote_start;
  }

  protected void raceFinished() throws SerialPortException {
    Message.LaneResult[] resultArray = result.toArray();
    result = null;
    timeOfFirstResult = 0;
    // raceFinished(LaneResult[]) may synchronously prepare a new heat,
    // overwriting result member, so clear that before calling
    raceFinished(resultArray);
    // Not sure what triggers it (possibly timer reset by switch?), but timer
    // sometimes sends a <n>=9.9999 for DNFs.  These don't have place info,
    // so don't match the early detector; instead they get put on the queue,
    // which we need to periodically drain.
    portWrapper.drain();
  }

  @Override
  public int getSafeNumberOfLanes() {
    return MAX_LANES;
  }

  public int getNumberOfLanes() throws SerialPortException {
    // Doesn't report actual number of lanes.
    return 0;
  }

  @Override
  protected void maskLanes(int lanemask) throws SerialPortException {
  }

  @Override
  public void prepareHeat(int roundid, int heat, int lanemask)
      throws SerialPortException {
    result = new TimerResult(lanemask);
    timeOfFirstResult = 0;
    super.prepareHeat(roundid, heat, lanemask);
  }

  @Override
  public void abortHeat() throws SerialPortException {
    result = null;
    timeOfFirstResult = 0;
    super.abortHeat();
  }

  @Override
  public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState)
      throws SerialPortException {
  }

  protected void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException {
    if (timeOfFirstResult != 0 && System.currentTimeMillis() - timeOfFirstResult > 5000) {
      raceFinished();
      logOverdueResults();
    }
  }
}
