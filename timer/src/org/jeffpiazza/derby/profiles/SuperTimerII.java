package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

public class SuperTimerII extends TimerDeviceWithProfile {
  public SuperTimerII(TimerPortWrapper portWrapper) {
    super(portWrapper, profile());
  }
  // Unimplemented handling/commands
  // ----------------------------------------------
  // Pause heat: 3@7A\r

  // Track timeout handling
  // 00\r (0x30 0x30 0x0d) is request track info/timeout. Char 2 of the return is the timeout
  //    \/--- Timeout char
  // 4a 7c 7f 42 43 41 45  J|BCAE // 15s timeout
  // 4a 70 7f 42 43 41 45  JpBCAE // 12s timeout

  // Set timeout command = 32 <@/0x40 + (4*num sec)> 0d
  // Example command and responses:
  // cmd (hex) (ascii) Timeout
  // 32 a4 0d    2¤.    25
  // 32 a0 0d    2 .    24
  // 32 3f 0d    2?.    21-23 // Not sure why these repeat
  // 32 90 0d    2.    20
  // 32 3f 0d    2?.    16-19 // Not sure why these repeat
  // 32 7c 0d    2|.    15
  // 32 78 0d    2x.    14
  // 32 74 0d    2t.    13
  // 32 70 0d    2p.    12
  // 32 6c 0d    2l.    11
  // 32 68 0d    2h.    10
  // 32 40 0d    2@.    NO Timeout

  public static Profile profile() {
    return Profile.forTimer("Super Timer II", "Super Timer II")
        .max_lanes(6)
        .gate_state_is_knowable(false)
        .decimal_insertion_location(-4)
        .end_of_line("\r")
        .prober("UUUUc\r", "UUUUt")
        .params(SerialPort.BAUDRATE_9600,
                  SerialPort.DATABITS_8,
                  SerialPort.STOPBITS_1,
                  SerialPort.PARITY_NONE)
        .setup("3O5A\r3O5A\r")
        .match("#([1-6])",
               Event.PARTIAL_LANE_RESULT_LANE_NUM, 1)
        .match("(\\d{4,})",
               Event.PARTIAL_LANE_RESULT_TIME, 0)
       // This is the pattern if it was one line. 
       //Since it is multiple, we have to use the partial results
       //  .match("PARTIAL([1-9])=((\\d\\.\\d+))",
       //         Event.LANE_RESULT, 1, 2)
        .match("!",
               Event.NO_MORE_RESULTS) // Signifies the end of the results messages from the timer.
        // Lane mask command is 3<mask>5A\r
        // The mask is the character that represents the binary sequence for the
        // desired lanes.
        // 3A5A\r	Lane 1 enabled       Binary: 000001
        // 3C5A\r	Lanes 1-2 enabled    Binary: 000011
        // 3G5A\r	Lanes 1-3 enabled    Binary: 000111
        // 3O5A\r	Lanes 1-4 enabled    Binary: 001111
        // 3_5A\r	Lanes 1-5 enabled    Binary: 011111
        // 3|5A\r	Lanes 3, 4, 5, 6     Binary: 111100
        .single_mask_heat_prep(null, null, "3", "5A\r", 64)
        ;
  }
}
