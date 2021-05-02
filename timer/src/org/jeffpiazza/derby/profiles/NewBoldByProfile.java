package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

public class NewBoldByProfile extends TimerDeviceWithProfile {
  public NewBoldByProfile(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("NewBold DT, TURBO, or DerbyStick")
        .params(SerialPort.BAUDRATE_1200,
                SerialPort.DATABITS_7,
                SerialPort.STOPBITS_2,
                SerialPort.PARITY_NONE)
        .setup(" ")
        .match("^\\s*(\\d)\\s+(\\d\\.\\d+)(\\s.*|)", Event.LANE_RESULT, 1, 2)
        .heat_prep(" ");
  }
}
