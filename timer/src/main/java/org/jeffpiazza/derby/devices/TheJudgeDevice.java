package org.jeffpiazza.derby.devices;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

/*
Race Number 14
Checking valid lanes....
Number of Lanes: 6
Ready to Start Race
Go
Lane 3    3.1234 seconds    Win
Lane 2    3.2345 seconds    Place
(...)

Race Over
 */
public class TheJudgeDevice extends TimerDeviceBase {
  public TheJudgeDevice(SerialPortWrapper portWrapper) {
    super(portWrapper);
  }

  public static String toHumanString() {
    return "The Judge (New Directions)";
  }

  // Experimental flags for explicitly resetting the timer on different
  // conditions.  The Judge resets itself ~25 seconds after a heat completes,
  // so there shouldn't really be any need to do an explicit reset at all.
  //
  // If resetOnReady is true, reset the timer upon receipt of a heat-ready
  // message from the web server.
  //
  // If resetOnRaceOver is true, reset the timer immediately after recognizing
  // the "Race Over" message from the timer itself.
  private static boolean resetOnReady = false;
  private static boolean resetOnRaceOver = false;

  private String timerIdentifier;

  // If true,
  public static void setResetOnReady(boolean v) {
    resetOnReady = v;
  }

  public static void setResetOnRaceOver(boolean v) {
    resetOnRaceOver = v;
  }

  // Since the device doesn't listen to anything we say, we don't "probe" the
  // device at all, we just assume that we're talking to one.
  @Override
  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE,
                                   /* rts */ false,
                                   /* dtr */ false)) {
      return false;
    }

    portWrapper.write("*");
    long deadline = System.currentTimeMillis() + 500;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.indexOf("Checking Valid Lanes") >= 0) {
        timerIdentifier = s;
        LogWriter.serial("* 'The Judge' detected.");
        setUp();
        return true;
      }
    }
    return false;
  }

  private void setUp() {
    portWrapper.registerEarlyDetector(new SerialPortWrapper.Detector() {
      public String apply(String line) throws SerialPortException {
        Matcher m = startRacePattern.matcher(line);
        if (m.find()) {
          raceStarted();
          return line.substring(m.end());
        }
        return line;
      }
    });
  }

  private int numberOfLanes = 0;
  private Message.LaneResult[] results;  // String time, int place
  private int lanesReceived = 0;  // Number of populated entries in results so far

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return numberOfLanes;
  }

  @Override
  public String getTimerIdentifier() { return timerIdentifier; }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    // resetOnReady means that we should send a reset signal to the timer
    // upon getting a new heat-ready message from the server
    if (resetOnReady && !(roundid == this.roundid && heat == this.heat)) {
      portWrapper.write("*");
    }
    // Nothing to be done with the lane mask
    prepare(roundid, heat);
  }

  @Override
  public void abortHeat() throws SerialPortException {
    roundid = heat = 0;
  }

  private static final Pattern numberOfLanesPattern = Pattern.compile(
      "Number of Lanes:?\\s+(\\d)");
  private static final Pattern startRacePattern = Pattern.compile("^G[oO]!?$");
  private static final Pattern singleLanePattern = Pattern.compile(
      "^Lane\\s+(\\d)\\s+(\\d+\\.\\d+)(\\s.*)?$");
  private static final Pattern raceOverPattern = Pattern.compile("Race Over.*");

  // Returns either a Matcher that successfully matched within line, or null.
  private static Matcher matched(Pattern pattern, String line) {
    Matcher m = pattern.matcher(line);
    if (m.find()) {
      return m;
    }
    return null;
  }

  @Override
  public void poll() throws SerialPortException, LostConnectionException {
    String line;
    while ((line = portWrapper.nextNoWait()) != null) {
      Matcher m;
      if ((m = matched(numberOfLanesPattern, line)) != null) {
        numberOfLanes = Integer.parseInt(m.group(1));
        LogWriter.trace(
            "Identified " + numberOfLanes + " lane(s).");
        System.out.println("Identified " + numberOfLanes + " lane(s).");  // TODO
      } else if (matched(startRacePattern, line) != null) {
        // TODO The startRacePattern should have been picked up in the
        // early detector; this is just insurance.
        LogWriter.trace("Race start detected by polling");
        raceStarted();
      } else if ((m = matched(singleLanePattern, line)) != null) {
        int lane = Integer.parseInt(m.group(1));
        if (results == null) {
          LogWriter.serial("* Unexpected race results before race start");
          if (numberOfLanes == 0) {
            LogWriter.serial("* Forcing 6 lanes");
            numberOfLanes = 6;
          }
          results = new Message.LaneResult[numberOfLanes];
        }
        if (lane > 0 && lane <= results.length) {
          results[lane - 1] = new Message.LaneResult();
          results[lane - 1].place = 1 + lanesReceived;
          String time = m.group(2);
          if (time.charAt(1) != '.') {
            // First char is a digit, second is normally "." unless more than
            // one digit ahead of decimal.  (A DNF result doesn't show up until
            // the timer expires, about 25 seconds or so.)  Cap times at just
            // under 10 seconds.
            time = "9.9999";
          }
          results[lane - 1].time = time;
        } else {
          LogWriter.serial("Can't record result for lane " + lane);
        }
        ++lanesReceived;
      } else if (matched(raceOverPattern, line) != null) {
        invokeRaceFinishedCallback(roundid, heat, results);
        roundid = heat = 0;
        if (resetOnRaceOver) {
          portWrapper.write("*");
        }
      }
    }
  }

  private void raceStarted() {
    System.out.println("* Race started!");  // TODO
    LogWriter.serial("* Race started!");  // TODO
    int nlanes = numberOfLanes;
    if (nlanes == 0) {
      nlanes = 6;
      LogWriter.serial("Lane count not received; forcing 6 lanes");
    }
    results = new Message.LaneResult[nlanes];
    lanesReceived = 0;
    invokeRaceStartedCallback();
  }
}
