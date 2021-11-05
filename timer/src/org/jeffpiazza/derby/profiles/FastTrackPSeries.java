package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

public class FastTrackPSeries extends TimerDeviceWithProfile {
  public FastTrackPSeries(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("FastTrack P-series")
        .params(SerialPort.BAUDRATE_9600,
                SerialPort.DATABITS_8,
                SerialPort.STOPBITS_1,
                SerialPort.PARITY_NONE)
        .max_lanes(6)
        .gate_state_is_knowable(false)
        .setup("RF")
        .match(" *([A-Z])=(\\d\\.\\d+)([^ ]?)", Event.LANE_RESULT, 1, 2);
  }
}
