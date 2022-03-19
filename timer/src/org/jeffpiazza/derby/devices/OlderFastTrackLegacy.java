package org.jeffpiazza.derby.devices;

import java.util.regex.Matcher;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

// This device class supports older FastTrack devices, i.e., P-series, that
// don't accept (i.e., are deaf to) commands from the host.
//
// From http://www.microwizard.com/faq.html:
//
// I want to write my own software. What is the format of the timer's serial interface?
// A. The timer sends this string when all the cars have finished.
//
// A=1.234! B=2.345 C=3.456 D=4.567 E=0.000 F=0.000
//
// It puts an exclamation point after the winning time, with 2 spaces after
// every time (the exclamation point takes up a space). Also if you only have a
// 4 lane track the unused lanes (E and F in this example) will show 0.000
//
// Other characters:
//
// It sends an "@" sign whenever the reset switch is closed. Also it sends a
// <LF> and a <CR> (That is ASCII code 10 and 13) at the end of each string.
//
// The serial settings to view the timer results yourself are 9600 baud, 8 bits,
// no parity, 1 stop bit, no flow control. With these settings you should be
// able to see the results in hyperterminal.
// Unpopulated/overdue lanes report 0.0 instead of 9.9999
public class OlderFastTrackLegacy extends TimerDeviceBase {
  public OlderFastTrackLegacy(SerialPortWrapper portWrapper) {
    super(portWrapper);
  }

  public static final int MAX_LANES = 6;

  public static String toHumanString() {
    return "FastTrack P-series";
  }

  @Override
  public boolean canBeIdentified() {
    return false;
  }

  // Since the device doesn't listen to anything we say, we don't "probe" the
  // device at all, we just assume that we're talking to one.
  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE)) {
      return false;
    }

    MicroWizard.readFeatures(portWrapper);

    setUp();
    return true;
  }

  protected void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
      public String apply(String line) throws SerialPortException {
        Matcher m = TimerDeviceUtils.matchedCommonRaceResults(line);
        if (m != null) {
          Message.LaneResult[] results
              = TimerDeviceUtils.extractResults(line, m.start(), m.end(),
                                                MAX_LANES);
          raceFinished(results);
          return line.substring(0, m.start()) + line.substring(m.end());
        } else {
          return line;
        }
      }
    });
    MicroWizard.registerEarlyDetectorForReset(portWrapper);
  }

  // No lane masks!
  public void prepareHeat(int roundid, int heat, int lanemask)
      throws SerialPortException {
    prepare(roundid, heat);
  }

  public void abortHeat() throws SerialPortException {
    prepare(0, 0);
  }

  protected void raceFinished(Message.LaneResult[] results)
      throws SerialPortException {
    invokeRaceFinishedCallback(roundid, heat,
                               TimerDeviceUtils.zeroesToNines(results));
  }

  public int getNumberOfLanes() throws SerialPortException {
    // FastTrack, at least older versions, doesn't report actual number of lanes.
    return 0;
  }

  public String getTimerIdentifier() {
    return null;
  }

  public void poll() throws SerialPortException, LostConnectionException {
    String line;
    while ((line = portWrapper.nextNoWait()) != null) {
      LogWriter.serial("Unexpected timer output: " + line);
    }
  }
}
