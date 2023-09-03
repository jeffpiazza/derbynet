package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

// This class supports PICmicro-based DIY timer designed by Bert Drake,
// described at http://drakedev.com/pinewood/.  The "Race Timer Software
// Interface Protocol" is described at
// http://drakedev.com/pinewood/protocol.html
public class BertDrake extends TimerDeviceWithProfile {
  public BertDrake(TimerPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("Bert Drake timer", "BertDrake")
        .params(SerialPort.BAUDRATE_9600,
                SerialPort.DATABITS_8,
                SerialPort.STOPBITS_1,
                SerialPort.PARITY_NONE)
        .gate_watcher("C", new Profile.Detector("^Gc$", Event.GATE_CLOSED),
                      new Profile.Detector("^Go$", Event.GATE_OPEN))
        .prober(new Profile.CommandSequence("R"), "V", "Bert Drake")
        .match("^B$", Event.GATE_OPEN)
        .match("^\\s*(\\d)\\s+(\\d\\.\\d+)(\\s.*|)", Event.LANE_RESULT, 1, 2)
        // TimerDeviceUtils.zeroesToNines(results)
        .heat_prep("R")
        .on(Event.OVERDUE, "F" /* TODO: , "T" */);
  }
}
