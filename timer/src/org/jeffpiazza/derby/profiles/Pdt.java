package org.jeffpiazza.derby.profiles;

import org.jeffpiazza.derby.timer.Profile;
import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

// TODO remote start "S"
public class Pdt extends TimerDeviceWithProfile {
  public Pdt(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("PDT timer (https://www.dfgtec.com/pdt)", "MiscJunk")
        .params(SerialPort.BAUDRATE_9600,
                SerialPort.DATABITS_8,
                SerialPort.STOPBITS_1,
                SerialPort.PARITY_NONE)
        .prober(new Profile.CommandSequence(""), "V", "vert=")
        .setup("R", "N" /* followed by numl=" */)
        .remote_start("S")
        .gate_watcher("G",
                      new Profile.Detector("O", Event.GATE_OPEN),
                      new Profile.Detector("\\.", Event.GATE_CLOSED))
        .match("numl=(\\d)", Event.LANE_COUNT, 1)
        .match("^B$", Event.RACE_STARTED)
        // .match("^K$"  )  // ready for next race?
        .match("(\\d) - (\\d+\\.\\d+)", Event.LANE_RESULT, 1, 2)
        .heat_prep("U", "M", '1')
        .on(Event.OVERDUE, "F");
  }

}
