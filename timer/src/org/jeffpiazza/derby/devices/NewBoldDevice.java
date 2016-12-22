package org.jeffpiazza.derby.devices;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.SerialPortWrapper;

// TURBO  NewBold Products
// 1 3.5109 3 3.1288 2 2.9831 4 3.5644
// 1 2.5455 2 2.6790 3 2.8820 4 3.5134
// ...
// Returns results in finish order, with each entry giving the lane and time.
// (shows 1st Place Lane #, 1st Place Time, 2nd Place Lane #, 2nd Place Time, ...)
public class NewBoldDevice extends TimerDeviceBase {
  public NewBoldDevice(SerialPortWrapper portWrapper) {
    super(portWrapper);
  }

  @Override
  public boolean canBeIdentified() {
    return false;
  }

  public static String toHumanString() {
    return "NewBold DT, TURBO, or DerbyStick";
  }

  @Override
  public boolean probe() throws SerialPortException {
    return portWrapper.port().setParams(SerialPort.BAUDRATE_1200,
                                        SerialPort.DATABITS_7,
                                        SerialPort.STOPBITS_2,
                                        SerialPort.PARITY_NONE,
                                        /* rts */ false,
                                        /* dtr */ false);
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return 0;
  }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    if (this.roundid == 0 && this.heat == 0) {
      // TODO: Sending a single SPC character would reset the timer; is that
      // desirable?
    }
    prepare(roundid, heat);
  }

  @Override
  public void abortHeat() throws SerialPortException {
    portWrapper.port().writeString(" ");  // Reset timer
  }

  private static final Pattern singleLanePattern = Pattern.compile(
      "^\\s+(\\d)\\s+(\\d\\.\\d+)(\\s.*)");

  @Override
  public void poll() throws SerialPortException, LostConnectionException {
    String line;
    ArrayList<Message.LaneResult> results = new ArrayList<Message.LaneResult>();
    while ((line = portWrapper.nextNoWait()) != null) {
      int nresults = 0;
      while (!line.isEmpty()) {
        Matcher m = singleLanePattern.matcher(line);
        if (m.find()) {
          int lane = Integer.getInteger(m.group(1));
          if (results.size() <= lane) {
            results.ensureCapacity(lane);
            while (results.size() <= lane) {
              results.add(null);
            }
          }
          results.set(lane, new Message.LaneResult());
          results.get(lane).place = 1 + nresults;
          results.get(lane).time = m.group(2);
          ++nresults;
          String msg = "*   Lane " + lane + ": " + m.group(2) + " seconds";
          System.out.println(msg);  // TODO
          portWrapper.logWriter().traceInternal(msg);  // TODO
        } else {
          String msg = "* Unrecognized: [[" + line + "]]";
          System.out.println(msg);
          portWrapper.logWriter().traceInternal(msg);
          break;
        }
      }
      if (nresults > 0) {
        invokeRaceFinishedCallback(roundid, heat,
                                   (Message.LaneResult[]) results.toArray());
        roundid = heat = 0;
        System.out.println("* Race finished!");  // TODO
        portWrapper.logWriter().traceInternal("* Race finished!");  // TODO
      }
    }
  }
}
