package org.jeffpiazza.derby.devices;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public class RacemasterLegacy extends TimerDeviceBase {
  /*
  Racemaster timers were produced by:
  JIT, Inc.
  Tony Weida

  [H[J JIT, Inc. Racemaster Software Copyright January 2002
  Software Version 0.0 Hardware Version 0.0
  Serial Port Functions:
  D - Display Zero Page Memory,
  V - Display Version and Help
  L - Report Finish Results to RS-232 Port
  T - Perform Display Test All Pixels Enabled
  O - Display Lane Order and State, LED Indicates Start Switch State
  Q - Display Rotating Character Display Test
  R - Restart Timing Sequence
  F - Flash LED
  S - Start Timing Sequence
   */
  private static final String DISPLAY_VERSION_AND_HELP = "V";
  private static final String REPORT_RESULTS = "L";
  // This actually starts the timer running, without waiting for the start gate.
  private static final String START_TIMING = "S";
  private static final String RESET_TIMER = "R";

  public RacemasterLegacy(SerialPortWrapper portWrapper) {
    super(portWrapper);
  }

  public static String toHumanString() {
    return "JIT Racemaster";
  }

  private static final int kDeadlineMs = 10000;

  private String timerIdentifier;
  private volatile TimerResult result;
  // The Racemaster doesn't give an indication when the timer starts, but it
  // does report each car crossing the finish.  When the first car crosses the
  // finish, we compute a deadline for the last car; if the deadline passes
  // before the last car cross, the missing cars are declared DNF.
  private volatile long lastCarDeadline;

  @Override
  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE)) {
      return false;
    }

    portWrapper.write(DISPLAY_VERSION_AND_HELP);
    long deadline = System.currentTimeMillis() + 2000;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.indexOf("JIT, Inc.") >= 0
          || s.indexOf("Racemaster Software") >= 0) {
        // This timer includes escape characters in some of its responses,
        // which can cause trouble when encoded as XML, so just filter them out.
        timerIdentifier = s.replace("\033", "");
        portWrapper.drain(deadline, 11);
        setUp();
        return true;
      }
    }

    return false;
  }

  protected void setUp() throws SerialPortException {
    // Enable reporting of the results over the serial port.  (Maybe.)
    // From the captured logs, it looks like L always triggers a
    // "Race Result Report" header.
    portWrapper.writeAndDrainResponse(REPORT_RESULTS, 2, 1000);
    portWrapper.registerEarlyDetector(new TimerDeviceUtils.SplittingDetector(
        new SerialPortWrapper.Detector() {
      @Override
      public String apply(String line) throws SerialPortException {
        boolean repeatReport = false;
        do {
          Matcher m0 = raceResultReportPattern.matcher(line);
          if (m0.find()) {
            line = line.substring(0, m0.start()) + line.substring(m0.end());
            repeatReport = true;
          }
        } while (repeatReport);

        Matcher m = singleLanePattern.matcher(line);
        if (m.find()) {
          int lane = m.group(1).charAt(0) - '0';
          String time = m.group(2);
          LogWriter.serial("Lane " + lane + ": " + time);
          if (result != null) {
            result.setLane(lane, time);
            if (result.isFilled()) {
              raceFinished();
            } else if (lastCarDeadline == 0) {
              lastCarDeadline = System.currentTimeMillis() + kDeadlineMs;
            }
          } else {
            LogWriter.serial("* No heat pending; ignoring time " + time
                + " for lane " + lane);
          }
          return line.substring(0, m.start()) + line.substring(m.end());
        }
        return line;
      }
    }));
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return 0;
  }

  @Override
  public String getTimerIdentifier() {
    return timerIdentifier;
  }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    prepare(roundid, heat);
    // Send R a few seconds after the heat is deemed complete, to reset the timer
    // (notwithstanding the "restart" description).  R seems always to produce
    // a report.
    result = new TimerResult(laneMask);
    lastCarDeadline = 0;
    portWrapper.writeAndDrainResponse(RESET_TIMER, 2, 1000);
  }

  private void raceFinished() throws SerialPortException {
    Message.LaneResult[] resultArray = result.toArray();
    result = null;
    lastCarDeadline = 0;
    invokeRaceFinishedCallback(roundid, heat, resultArray);
    roundid = heat = 0;
    portWrapper.writeAndDrainResponse(RESET_TIMER, 2, 1000);
  }

  @Override
  public void abortHeat() throws SerialPortException {
    portWrapper.writeAndDrainResponse(RESET_TIMER, 2, 1000);
  }

  /*
     First   Place Single Lane Number:    4    Time in Seconds:    4.3726
     Second  Place Single Lane Number:    3    Time in Seconds:    4.4070
     Third   Place Single Lane Number:    1    Time in Seconds:    4.4974
     Fourth  Place Single Lane Number:    2    Time in Seconds:   18.5514
   */
  private static final Pattern singleLanePattern = Pattern.compile(
      "^.*Place Single Lane Number:\\s*(\\d+)\\s+Time in Seconds:\\s*(\\d+\\.\\d{4,})");
  private static final Pattern raceResultReportPattern = Pattern.compile(
      ".?\\[H.\\[J Race Master Race Result Report");

  @Override
  public void poll() throws SerialPortException, LostConnectionException {
    String line;
    while ((line = portWrapper.nextNoWait()) != null) {
      LogWriter.serial("POLL ignoring \"" + line + "\"");
    }
    if (lastCarDeadline != 0 && System.currentTimeMillis() >= lastCarDeadline) {
      LogWriter.serial("POLL timing out last car(s)");
      raceFinished();
    }
  }
}
