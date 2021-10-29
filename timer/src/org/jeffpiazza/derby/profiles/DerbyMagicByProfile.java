package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

// http://www.derbymagic.com/files/Timer.pdf
// http://www.derbymagic.com/files/GPRM.pdf
public class DerbyMagicByProfile extends TimerDeviceWithProfile {
  public DerbyMagicByProfile(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("Derby Magic timer")
        .max_lanes(8)
        .gate_state_is_knowable(false)
        .params(SerialPort.BAUDRATE_19200,
                  SerialPort.DATABITS_8,
                  SerialPort.STOPBITS_1,
                  SerialPort.PARITY_NONE)
        .prober("V", "Derby Magic")
        .setup("R")
        .match("^B$", Event.RACE_STARTED)
        .match("([1-8])=(\\d\\.\\d+)([!-/:-@])? *",
               Event.LANE_RESULT, 1, 2, 3)
        .remote_start("S");
  }
}
