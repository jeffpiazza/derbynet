package org.jeffpiazza.derby.profiles;

import org.jeffpiazza.derby.timer.Profile;
import jssc.SerialPort;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;

public class Champ extends TimerDeviceWithProfile {
  public Champ(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("\"The Champ\" (SmartLine/BestTrack)")
        .params(SerialPort.BAUDRATE_9600,
                  SerialPort.DATABITS_8,
                  SerialPort.STOPBITS_1,
                  SerialPort.PARITY_NONE)
        .end_of_line("\r")
        .max_lanes(6)
        // Pre-probe lets the timer settle
        .prober(new Profile.CommandSequence(""),
                  "v", "eTekGadget SmartLine Timer")
        .setup("r" /* RESET */,
                 // Interrogate for a bunch of details here
                 "or" /* read auto-reset */,
                 "ol" /* read lane character */,
                 "od" /* read decimal places */,
                 "op" /* read place character */,
                 "rs" /* read start switch */,
                 // Set to known state
                 "ol0" /* report lane 1 as "A" */,
                 "op3" /* set place character '!' */)
        .setup("on", new Profile.Detector("^(\\d)$", Event.LANE_COUNT))
        .max_running_time_ms(11000)
        .heat_prep("om0", "om", '1')
        .match(" *([A-Z])=(\\d\\.\\d+)([^ ]?)", Event.LANE_RESULT, 1, 2)
        .gate_watcher("rs" /* READ_START_SWITCH */,
                        new Profile.Detector("^0$", Event.GATE_CLOSED),
                        new Profile.Detector("^1$", Event.GATE_OPEN))
        .on(Event.RACE_STARTED, "rg")
        .on(Event.OVERDUE, "ra");
  }
}
