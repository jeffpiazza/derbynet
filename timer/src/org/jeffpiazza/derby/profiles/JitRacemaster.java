package org.jeffpiazza.derby.profiles;

/*
  Racemaster timers were produced by:
  JIT, Inc.
  Tony Weida

  [H[J JIT, Inc. Racemaster Software Copyright January 2002
  Software Version 0.0 Hardware Version 0.0
  Serial Port Functions:
  D - Display Zero Page Memory,
  V - Display Version and Help
  L - Report Finish Results to RS-232 Port
  T - Perform Display Test All Pixels Enabled
  O - Display Lane Order and State, LED Indicates Start Switch State
  Q - Display Rotating Character Display Test
  R - Restart Timing Sequence
  F - Flash LED
  S - Start Timing Sequence
 */

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

public class JitRacemaster extends TimerDeviceWithProfile {
  public JitRacemaster(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    return Profile.forTimer("JIT Racemaster", "JIT")
        .params(SerialPort.BAUDRATE_9600,
                SerialPort.DATABITS_8,
                SerialPort.STOPBITS_1,
                SerialPort.PARITY_NONE)
        .gate_state_is_knowable(false)
        // TODO filter escape characters from timer identifier string
        .prober("V", "JIT, Inc.*Racemaster Software", "^Software Version")
        .setup("L")
        .match("^.*Place Single Lane Number:\\s*(\\d+)\\s+Time in Seconds:\\s*(\\d+\\.\\d{4,})",
               Event.LANE_RESULT, 1, 2)
        .heat_prep("R");
  }
}
