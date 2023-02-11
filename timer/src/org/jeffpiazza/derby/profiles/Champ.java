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
    return Profile.forTimer("\"The Champ\" (SmartLine/BestTrack)", "TheChamp")
        .params(SerialPort.BAUDRATE_9600,
                  SerialPort.DATABITS_8,
                  SerialPort.STOPBITS_1,
                  SerialPort.PARITY_NONE)
        .end_of_line("\r")
        .max_lanes(6)
        // Pre-probe lets the timer settle
        .prober(new Profile.CommandSequence(""),
                // January, 2023: New "Champ" timer responds with
                // Rev H Copyright SRM Enterprises
                // (not even a valid copyright)
                  "v", "eTekGadget SmartLine Timer")
        .setup("r" /* RESET */,
               // The ox0 appears to cause the timer to stop responding for a
               // second or two, ignoring at least all the commands that follow
               // in this set-up.
               /* "ox0",  Set Champ Timer mode instead of DTX */
               // Interrogate for a bunch of details here
               "or" /* read auto-reset */,
               "ol" /* read lane character */,
               "od" /* read decimal places */,
               "op" /* read place character */,
               "rs" /* read start switch */,
               // Set to known state
               "ol0" /* report lane 1 as "A" */,
               "op3" /* set place character '!' */)
        .setup("on", new Profile.Detector("^(\\d)$", Event.LANE_COUNT, 1))
        .heat_prep("om0", "om", '1')
        .match(" *([A-Z])=(\\d\\.\\d+)([^ ]?)", Event.LANE_RESULT, 1, 2)
        .gate_watcher("rs" /* READ_START_SWITCH */,
                        new Profile.Detector("^0$", Event.GATE_CLOSED),
                        new Profile.Detector("^1$", Event.GATE_OPEN))
        // rg = "Return results when race ends"
        // Any gate polling sent after this apparently causes the 'rg' to be
        // canceled.  Also, Pack936 reported an issue with the 'rg' being sent
        // too quickly after gate opening, causing the previous heat's results
        // to be sent again.
        .on(Event.RACE_STARTED, "rg")
        // ra = "Force end of race, return results, then reset"
        .on(Event.OVERDUE, "ra");
  }
}
