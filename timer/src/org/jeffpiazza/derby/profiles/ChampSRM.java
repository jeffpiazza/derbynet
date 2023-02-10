package org.jeffpiazza.derby.profiles;

import org.jeffpiazza.derby.timer.Profile;
import jssc.SerialPort;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;

public class ChampSRM extends TimerDeviceWithProfile {
  public ChampSRM(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("\"The Champ\" (SRM)", "TheChampSRM")
        .params(SerialPort.BAUDRATE_9600,
                  SerialPort.DATABITS_8,
                  SerialPort.STOPBITS_1,
                  SerialPort.PARITY_NONE)
        .end_of_line("\r")
        // January, 2023: New "Champ" timer responds to a "v" with:
        // Rev H Copyright SRM Enterprises
        // (not even a valid copyright)
        // Pre-probe lets the timer settle
        .prober(new Profile.CommandSequence(""), "v", "SRM.*Enterprises")
        // This firmware responds "?" to each one-letter command EXCEPT:
        // "A", "C" (gives gate state as 0 or 1), "D", "F", "R", and "v" (not "V")
        .gate_state_is_knowable(false)
        .max_lanes(6)
        .match("S", Event.RACE_STARTED)
        // The original Champ timer can report lanes with letters or numbers.
        // This firmware reports with numbers by default, and we haven't figured
        // out if it's configurable.
        .match(" *(\\d)=(\\d\\.\\d+)([^ ]?)", Event.LANE_RESULT, 1, 2)
        ;
  }
}
