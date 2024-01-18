package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

// This class supports the "Derby Timer" device, http://derbytimer.com
public class DerbyTimer extends TimerDeviceWithProfile {
  public DerbyTimer(TimerPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("Derby Timer", "DerbyTimer.com")
        .params(SerialPort.BAUDRATE_9600,
                SerialPort.DATABITS_8,
                SerialPort.STOPBITS_1,
                SerialPort.PARITY_NONE)
        .prober("R", "^RESET$", "^READY\\s*(\\d+)\\s+LANES")
        .match("^READY\\s*(\\d+)\\s+LANES", Event.LANE_COUNT, 1)
        .match("^\\s*(\\d)\\s+(\\d\\.\\d+)(\\s.*|)", Event.LANE_RESULT, 1, 2)
        .match("^RACE$", Event.RACE_STARTED)
        //.match("^FINISH$", Event.RACE_FINISHED)
        //TimerDeviceUtils.zeroesToNines(results)
        .heat_prep("C", "M", '1')
        .gate_watcher("G",
                      new Profile.Detector("^U$", Event.GATE_CLOSED),
                      new Profile.Detector("^D$", Event.GATE_OPEN))
        .on(Event.OVERDUE, "F");
  }
}
