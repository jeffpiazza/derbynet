package org.jeffpiazza.derby.devices;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.SerialPortWrapper;

/*

Race Number 14
Checking valid lanes....
Number of Lanes: 6
Ready to Start Race
GO!
Lane 3    3.1234 seconds    Win
Lane 2    3.2345 seconds    Place
(...)

Race Over

 */
// TODO: EXPERIMENTAL!
public class TheJudgeDevice extends TimerDeviceBase {
  public TheJudgeDevice(SerialPortWrapper portWrapper) {
    super(portWrapper);
  }

  public static String toHumanString() {
    return "The Judge (New Directions)";
  }

  @Override
  public boolean canBeIdentified() {
    return false;
  }

  // Since the device doesn't listen to anything we say, we don't "probe" the
  // device at all, we just assume that we're talking to one.
  @Override
  public boolean probe() throws SerialPortException {
    return portWrapper.port().setParams(SerialPort.BAUDRATE_9600,
                                        SerialPort.DATABITS_8,
                                        SerialPort.STOPBITS_1,
                                        SerialPort.PARITY_NONE,
                                        /* rts */ false,
                                        /* dtr */ false);
  }

  private int numberOfLanes = 0;
  private Message.LaneResult[] results;  // String time, int place
  private int lanesReceived = 0;  // Number of populated entries in results so far

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return numberOfLanes;
  }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    // Since we can't talk to the timer, nothing to do with the lane mask
    prepare(roundid, heat);
  }

  @Override
  public void abortHeat() throws SerialPortException {
    roundid = heat = 0;
  }

  private static final Pattern numberOfLanesPattern = Pattern.compile(
      "Number of Lanes.?\\s+(\\d)");
  private static final Pattern startRacePattern = Pattern.compile("GO!");
  private static final Pattern singleLanePattern = Pattern.compile(
      "^Lane (\\d)\\s+(\\d\\.\\d+)(\\s.*)$");
  private static final Pattern raceOverPattern = Pattern.compile("Race Over.*");

  // Returns either a Matcher that successfully matched within line, or null.
  private Matcher matched(Pattern pattern, String line) {
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
        numberOfLanes = Integer.getInteger(m.group(1));
        portWrapper.logWriter().traceInternal(
            "Identified " + numberOfLanes + " lane(s).");  // TODO
        System.out.println("Identified " + numberOfLanes + " lane(s).");  // TODO
      } else if (matched(startRacePattern, line) != null) {
        int nlanes = numberOfLanes;
        if (nlanes == 0) {
          nlanes = 6;
          portWrapper.logWriter().traceInternal(
              "Lane count not received; forcing 6 lanes");
        }
        results = new Message.LaneResult[nlanes];
        lanesReceived = 0;
        invokeRaceStartedCallback();
        System.out.println("* Race started!");  // TODO
        portWrapper.logWriter().traceInternal("* Race started!");  // TODO
      } else if ((m = matched(singleLanePattern, line)) != null) {
        int lane = Integer.getInteger(m.group(1));
        results[lane] = new Message.LaneResult();
        results[lane].place = 1 + lanesReceived;
        results[lane].time = m.group(2);
        ++lanesReceived;
        System.out.println("*   Lane " + lane + ": " + m.group(2) + " seconds");  // TODO
        portWrapper.logWriter().traceInternal(
            "*   Lane " + lane + ": " + m.group(2) + " seconds");  // TODO
      } else if (matched(raceOverPattern, line) != null) {
        invokeRaceFinishedCallback(roundid, heat, results);
        roundid = heat = 0;
        System.out.println("* Race finished!");  // TODO
        portWrapper.logWriter().traceInternal("* Race finished!");  // TODO
      }
    }
  }
}
