package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

public class TheJudge extends TimerDeviceWithProfile {
  public TheJudge(TimerPortWrapper portWrapper) {
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
        // DNFs get reported as e.g.
        // +23:23:27.919S		<-- Lane 6     31.0589   DNF
        // i.e., they have a time just like every other lane, reported just before
        // the Race Over message.
        .match("^Lane\\s+(\\d)\\s+0*(\\d+\\.\\d+)(\\s.*)?$",
               Event.LANE_RESULT, 1, 2)
        // "Race Over" seems to be redundant -- DNFs get reported as lane results,
        // and the last lane result will trigger a RACE_FINISHED event.
        .match("Race Over.*", Event.RACE_FINISHED)
        .heat_prep("om0", "om", '1')
        // Since gate state is not knowable, gate_watcher doesn't run, so this
        // shouldn't make any difference.  It's not even clear the timer supports
        // this command.
        .gate_watcher("rs" /* READ_START_SWITCH */)
        .on(Event.OVERDUE, "*")
        .end_of_line("\r");
  }
}
