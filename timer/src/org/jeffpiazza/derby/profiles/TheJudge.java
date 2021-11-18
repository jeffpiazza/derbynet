package org.jeffpiazza.derby.profiles;

import org.jeffpiazza.derby.timer.Profile;
import jssc.SerialPort;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;

public class TheJudge extends TimerDeviceWithProfile {
  public TheJudge(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("The Judge (New Directions)", "TheJudge")
        .params(SerialPort.BAUDRATE_9600,
                SerialPort.DATABITS_8,
                SerialPort.STOPBITS_1,
                SerialPort.PARITY_NONE)
        .gate_state_is_knowable(false)
        .prober("*", "Checking Valid Lanes")
        .setup() // No set-up commands
        .match("Number of Lanes:?\\s+(\\d)", Event.LANE_COUNT, 1)
        .match("^G[oO]!?$", Event.RACE_STARTED)
        .match("^Lane\\s+(\\d)\\s+0*(\\d+\\.\\d+)(\\s.*)?$",
               Event.LANE_RESULT, 1, 2)
        .match("Race Over.*", Event.RACE_FINISHED)
        .heat_prep("om0", "om", '1')
        .gate_watcher("rs" /* READ_START_SWITCH */)
        .on(Event.OVERDUE, "*")
        .end_of_line("\r");
  }
}
