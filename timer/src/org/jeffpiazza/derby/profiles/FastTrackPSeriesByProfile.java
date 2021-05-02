package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

public class FastTrackPSeriesByProfile extends TimerDeviceWithProfile {
  public FastTrackPSeriesByProfile(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("FastTrack P-series")
        .params(SerialPort.BAUDRATE_9600,
                SerialPort.DATABITS_8,
                SerialPort.STOPBITS_1,
                SerialPort.PARITY_NONE)
        .max_lanes(6)
        .setup("RF")
        .match("([A-Z]=(\\d\\.\\d+).?)( [A-Z]=(\\d\\.\\d+).?)*$",
               new Profile.Detector(
                   " *([A-Z])=(\\d\\.\\d+)([^ ]?)",
                   Event.LANE_RESULT, 1, 2));
  }
}
