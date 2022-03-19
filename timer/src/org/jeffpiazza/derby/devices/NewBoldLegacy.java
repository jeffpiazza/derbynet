package org.jeffpiazza.derby.devices;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

// TURBO  NewBold Products
// 1 3.5109 3 3.1288 2 2.9831 4 3.5644
// 1 2.5455 2 2.6790 3 2.8820 4 3.5134
// ...
// Returns results in finish order, with each entry giving the lane and time.
// (shows 1st Place Lane #, 1st Place Time, 2nd Place Lane #, 2nd Place Time, ...)
public class NewBoldLegacy extends TimerDeviceBase {
  public NewBoldLegacy(SerialPortWrapper portWrapper) {
    super(portWrapper);
  }

  // Once a race completes, we need to send a timer reset to prepare for the
  // next race.  We could do that immediately upon conclusion of the race, but
  // then the results don't display long enough to be seen.  Instead, we set
  // this deadline value, and check it during the poll loop.
  //
  // < 0 means no reset is pending.
  private long timerResetMillis = -1;

  @Override
  public boolean canBeIdentified() {
    return false;
  }

  public static String toHumanString() {
    return "NewBold DT, TURBO, or DerbyStick";
  }

  @Override
  public boolean probe() throws SerialPortException {
    if (portWrapper.setPortParams(SerialPort.BAUDRATE_1200,
                                  SerialPort.DATABITS_7,
                                  SerialPort.STOPBITS_2,
                                  SerialPort.PARITY_NONE)) {
      portWrapper.write(" ");  // Reset timer
      return true;
    }
    return false;
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return 0;
  }

  @Override
  public String getTimerIdentifier() { return null; }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    // if (this.roundid == 0 && this.heat == 0) { ... }
    prepare(roundid, heat);
  }

  @Override
  public void abortHeat() throws SerialPortException {
    portWrapper.write(" ");  // Reset timer
  }

  private static final Pattern singleLanePattern = Pattern.compile(
      "^\\s*(\\d)\\s+(\\d\\.\\d+)(\\s.*|)");

  @Override
  public void poll() throws SerialPortException, LostConnectionException {
    String line;
    ArrayList<Message.LaneResult> results = new ArrayList<Message.LaneResult>();
    while ((line = portWrapper.nextNoWait()) != null) {
      int nresults = 0;
      while (!line.isEmpty()) {
        Matcher m = singleLanePattern.matcher(line);
        if (m.find()) {
          int lane = Integer.parseInt(m.group(1));
          String time = m.group(2);
          if (lane != 0) {
            // For DNF lanes, DerbyStick reports lane 0 and 0.0000 result.
            TimerDeviceUtils.addOneLaneResult(lane, time, nresults, results);
            LogWriter.serial("Lane " + lane + ": " + time + " seconds");
          } else {
            LogWriter.serial("DNF result");
          }
          nresults++;
          line = m.group(3).trim();
        } else {
          LogWriter.serial("* Unrecognized: [[" + line + "]]");
          break;
        }
      }
      // If there are only DNFs, then the race is
      if (nresults > 0) {
        invokeRaceFinishedCallback(roundid, heat,
                                   (Message.LaneResult[]) results.toArray(
                                       new Message.LaneResult[results.size()]));
        roundid = heat = 0;
        LogWriter.serial("Race finished!");
        timerResetMillis = System.currentTimeMillis()
            + Flag.delay_reset_after_race.value() * 1000;
      }
    }
    if (timerResetMillis > 0 && System.currentTimeMillis() > timerResetMillis) {
      portWrapper.write(" ");  // Reset timer
      timerResetMillis = -1;
    }
  }
}
