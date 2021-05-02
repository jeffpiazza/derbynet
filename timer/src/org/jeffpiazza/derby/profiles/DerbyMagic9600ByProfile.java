package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

public class DerbyMagic9600ByProfile extends TimerDeviceWithProfile {
  public DerbyMagic9600ByProfile(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return DerbyMagicByProfile.profile().params(SerialPort.BAUDRATE_9600,
                                                SerialPort.DATABITS_8,
                                                SerialPort.STOPBITS_1,
                                                SerialPort.PARITY_NONE);
  }
}
