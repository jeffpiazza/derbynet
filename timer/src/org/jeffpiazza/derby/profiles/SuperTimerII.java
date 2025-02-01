package org.jeffpiazza.derby.profiles;

import jssc.SerialPort;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

/* SuperTimer II Overview

The SuperTimer doesn't report a gate state or race started condition.
The SuperTimer race operates as follows:
- Send Masking command (no response from track)
- With Masking command, send the timeout if needed (How long track waits
  before reporting results after the gate opens)
- Run the race on the track by triggering the safety (track start), then the
  start button at the track end.
- The track will timeout after the timeout seconds (I believe default is 10s)
- Once timed out or all lanes have a time, the results are sent to the timer.
  The results are returned in finish order, reporting only lanes that finished.
  Example result for 4 lanes with a DNF:
      +20:28:05.495S		<-- #1
      +20:28:05.532S		<-- 19491
      +20:28:05.572S		<-- #3
      +20:28:05.609S		<-- 20376
      +20:28:05.536S		<-- #2
      +20:28:05.572S		<-- 27563
      +20:28:05.656S		<-- !
    These times represent Lane 1: 1.9491s, Lane 2: 2.7563s,
    Lane 3: 2.0376s, Lane 4: DNF

  To handle this, times are submitted in parts, PARTIAL_LANE_RESULT_LANE_NUM,
  then PARTIAL_LANE_RESULT_TIME (time gets the decimal inserted), then
  NO_MORE_RESULTS is submitted on the "!" to tell the timer software that any 
  un-reported lanes are to be marked as a DNF.
  The NO_MORE_RESULTS will fill in all times Aas DNF, triggering the 
  RACE_FINISHED event internally and posting the times.
*/



public class SuperTimerII extends TimerDeviceWithProfile {
  public SuperTimerII(TimerPortWrapper portWrapper) {
    super(portWrapper, profile());
  }
  // ----------------------------------------------
  // Unimplemented handling/commands
  // ----------------------------------------------
  // Pause heat: 3@7A\r

  // Track timeout handling
  // Timeout -- GET
  //     This command requests the timeout info from the track. It would be nice to update the
  //     value in the web UI to show that.
  //     00\r (0x30 0x30 0x0d) is request track info/timeout. Char 2 of the return is the timeout
  //        \/--- Timeout char
  //     4a 7c 7f 42 43 41 45  J|BCAE // 15s timeout
  //     4a 70 7f 42 43 41 45  JpBCAE // 12s timeout

  // Timeout -- SET
  //     Set timeout command 
  //         HEX = 32 <@/0x40 + (4*num sec)> 0d
  //         DEC = 2 <64 + 4 * num sec> \r
  //     Example command and responses:
  //     cmd (hex) (ascii) Timeout
  //     32 a4 0d    2¤.    25
  //     32 a0 0d    2 .    24
  //     32 3f 0d    2?.    21-23 // Not sure why these repeat
  //     32 90 0d    2.    20
  //     32 3f 0d    2?.    16-19 // Not sure why these repeat
  //     32 7c 0d    2|.    15
  //     32 78 0d    2x.    14
  //     32 74 0d    2t.    13
  //     32 70 0d    2p.    12
  //     32 6c 0d    2l.    11
  //     32 68 0d    2h.    10
  //     32 40 0d    2@.    NO Timeout
  // ----------------------------------------------

  public static Profile profile() {
    return Profile.forTimer("Super Timer II", "Super Timer II")
        .max_lanes(6)
        .gate_state_is_knowable(false)
        .decimal_insertion_location(-4)
        .timer_controls_timeout(true)
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
        
        // TODO: Implement this with the math for the time.
        // Can't figure out how to handle this with the math that has to happen as well
        // I'll leave this for future problems. The SuperTimer should default to 10s,
        // but it is stored in internal memory it seems, so I cannot verify this.
        // Command should be: 2(math:64 + 4*<time>>\r
        .set_heat_timeout_duration_command("2(math:64+<time>+<time>+<time>+<time>)")
        ;
  }
}
