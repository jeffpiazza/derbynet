package org.jeffpiazza.derby.devices;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

// TheJudge sends newlines at the start of a line, rather than at the end, which
// means we have to do all the matching in the early detector.
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
public class TheJudgeLegacy extends TimerDeviceBase {
  public TheJudgeLegacy(SerialPortWrapper portWrapper) {
    super(portWrapper);
  }

  public static String toHumanString() {
    return "The Judge (New Directions)";
  }

  private long resetAt = 0;
  private String timerIdentifier;

  // Since the device doesn't listen to anything we say, we don't "probe" the
  // device at all, we just assume that we're talking to one.
  @Override
  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE)) {
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

  private static final Pattern numberOfLanesPattern = Pattern.compile(
      "Number of Lanes:?\\s+(\\d)");
  private static final Pattern startRacePattern = Pattern.compile("^G[oO]!?$");
  private static final Pattern singleLanePattern = Pattern.compile(
      "^Lane\\s+(\\d)\\s+0*(\\d+\\.\\d+)(\\s.*)?$");
  private static final Pattern raceOverPattern = Pattern.compile("Race Over.*");

  private void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
      public String apply(String line) throws SerialPortException {
        Matcher m;
        if (matched(startRacePattern, line) != null) {
          raceStarted();
          return "";
        } else if ((m = matched(singleLanePattern, line)) != null) {
          int lane = Integer.parseInt(m.group(1));
          if (result == null) {
            LogWriter.serial("* Unexpected race results before race start");
            if (numberOfLanes == 0) {
              LogWriter.serial("* Forcing 6 lanes");
              numberOfLanes = 6;
            }
            result = new TimerResult((1 << numberOfLanes) - 1);
          }

          String time = m.group(2);
          if (time.charAt(1) != '.') {
            // First char is a digit, second is normally "." unless more than
            // one digit ahead of decimal.  (A DNF result doesn't show up until
            // the timer expires, about 25 seconds or so.)  Cap times at just
            // under 10 seconds.
            LogWriter.info("Converting " + time + " to 9.9999");
            time = "9.9999";
          }
          result.setLane(lane, time);
          if (result.isFilled()) {
            invokeRaceFinishedCallback(roundid, heat, result.toArray());
            roundid = heat = 0;
          }
          return "";
        } else if (matched(raceOverPattern, line) != null) {
          // invokeRaceFinishedCallback(roundid, heat, result.toArray());
          roundid = heat = 0;
          return "";
        }

        return line;
      }
    });
  }

  private int numberOfLanes = 0;
  private TimerResult result;

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return numberOfLanes;
  }

  @Override
  public String getTimerIdentifier() {
    return timerIdentifier;
  }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    result = new TimerResult(laneMask);
    prepare(roundid, heat);
  }

  @Override
  public void abortHeat() throws SerialPortException {
    result = null;
    roundid = heat = 0;
    resetAt = 0;
  }

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
    if (resetAt != 0 && resetAt <= System.currentTimeMillis()) {
      LogWriter.serial("Sending reset 10s after race start");
      portWrapper.write("*");
      resetAt = 0;
    }
    String line;
    while ((line = portWrapper.nextNoWait()) != null) {
      Matcher m;
      if ((m = matched(numberOfLanesPattern, line)) != null) {
        numberOfLanes = Integer.parseInt(m.group(1));
        LogWriter.serial("Identified " + numberOfLanes + " lane(s).");
      } else {
        LogWriter.serial("Ignored: \"" + line + "\"");
      }
    }
  }

  private void raceStarted() {
    if (numberOfLanes == 0) {
      numberOfLanes = 6;
      LogWriter.serial("Lane count not received; forcing 6 lanes");
    }
    invokeRaceStartedCallback();
    if (Flag.reset_after_start.value() != 0) {
      resetAt = System.currentTimeMillis()
          + 1000 * Flag.reset_after_start.value();
    }
  }
}
