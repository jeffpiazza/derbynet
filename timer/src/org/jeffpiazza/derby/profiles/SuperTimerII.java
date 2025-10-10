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

  public static Profile profile() {
    return Profile.forTimer("Super Timer II", "Super Timer II")
        .params(SerialPort.BAUDRATE_9600,
                  SerialPort.DATABITS_8,
                  SerialPort.STOPBITS_1,
                  SerialPort.PARITY_NONE)
        .max_lanes(6)
        .gate_state_is_knowable(false)
        .timer_scale_factor(1000)
        .end_of_line("\r")
        .prober("UUUUc", "UUUUt")
        // .setup("3O5A\r3O5A")
        .match("#([1-6])", Event.PARTIAL_LANE_RESULT_LANE_NUM, 1)
        .match("(\\d{4,})", Event.PARTIAL_LANE_RESULT_TIME, 0)
        .match("!", Event.RACE_FINISHED)
        // SuperTimer accepts the lane mask as a single command string, rather than
        // separate commands for each masked lane.  OneCommandMask is an alternative
        // to the unmask/mask/lane triple found in the HeatPreparation struct.
        //
        // 3A5A\r	Lane 1 enabled       Binary: 000001
        // 3C5A\r	Lanes 1-2 enabled    Binary: 000011
        // 3G5A\r	Lanes 1-3 enabled    Binary: 000111
        // 3O5A\r	Lanes 1-4 enabled    Binary: 001111
        // 3_5A\r	Lanes 1-5 enabled    Binary: 011111
        // 3\5A\r	Lanes 3, 4, 5, 6     Binary: 111100

        .embedded_mask_command(/* "3@5A" as an int: */0x33403541, 16, 4)

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

        //////////////////////////
        // TODO .set_heat_timeout_duration_command(new EmbeddedFieldCommand(0x3240, 2, 2))
        ;
  }

  // Additional insights reported by jeffmahoney in
  // https://github.com/jeffpiazza/derbynet/pull/386:
  //
  // The SuperTimer II has an undocumented protocol which the manufacturer
  // refuses to support outside of their own (very) outdated software.
  //
  // Here's what I've been able to gather from sniffing the serial line.
  // Commands from the host are completed with a CR ('\r').  Responses
  // from the timer may be completed with a CR.  Some are not.
  //
  // Initial detection:
  // Host sends "UUUUc\r"
  // Timer returns "UUUUt\r"
  //
  // Audio test;
  //   To perform an audio test where the timer will go through all of
  //   the announcements, send the command: "6\xa4\r"
  //
  // Configuration Query:
  // Host sends "00\r" (two zeroes)
  // Timer returns 6 characters, which do -not- include a \r.
  // 0: Tiebreaker Margin
  // 1: Timeout
  // 2: Lane Mask
  // 3: Announcement Style
  // 4: Unknown
  // 5: Unknown
  // 6: Unknown
  // Note: This may also function as a reset of sorts.  The Windows software
  // did it periodically.
  //
  // There are several classes of configuration, each prefixed with a digit.
  // Tiebreaker margin.
  //    If two cars finish within this margin, it is considered a tie.
  //    The available offsets are from 0-2.0ms in 0.1ms increments.
  //    The values start at 0x40 to indicate 0 and end at 0x54 to indicate
  //    2.0ms.  These values are represented in ASCII: @ through T.
  // Timeout select.
  //    If a car doesn't finish before the amount of time specified, a 10.000
  //    values is returned for that lane and the race is over.  The available
  //    options are from 0 (disabled) to 60 seconds.  0-47 seconds can be
  //    specified individually with binary values starting at 0x40 and
  //    incrementing 4 at a time until 47 is specified with 0xf4.  0 seconds
  //    is specified with 0x40 (@), 1 second is specified with 0x44 (D).
  //    10 seconds, which we use above, is 0x68 (h).  Starting with 48
  //    seconds, it gets strange and I'm uncertain if some of these values
  //    are bugs in the original software or if the protocol really does this.
  //    I haven't tested to see what timeout is actually used when there is
  //    a conflict.
  //    48 -> \x41 (A)
  //    49 -> \x41 (A)
  //    50 -> \x43 (C)
  //    51 -> \x43 (C)
  //	52 -> \xd0	 -- This conflicts with 36 seconds
  //	53 -> \x45 (E)
  //	54 -> \x45 (E)
  //	55 -> \x47 (G)
  //	56 -> \x47 (G)
  //	57 -> \x48 (H) -- This conflicts with 2 seconds
  //	58 -> \x49 (I)
  //	59 -> \x49 (I)
  //	60 -> \x49 (I)
  // Lane mask.
  //    The SuperTimer II supports up to 6 lanes.
  //    The value used is a 6 bit bitmask.  The timer itself is unaware
  //    of how many lanes are attached.  It is just instructed to monitor
  //    lanes.  If this is misconfigured, timeouts will always be triggered.
  //    The bitmask is offset by 0x40, so that an empty mask is @.
  //    For a 6 lane track: 0b111111 -> 'O'
  //    For a 4 lane track: 0b001111 -> 'O'
  //    For a 2 lane track: 0b000011 -> '\x7f'
  //    The bitmask can be sparse as well, so a 6 lane track using every
  //    other lane would be: 0b010101 -> 'U'
  // Announcement mode.
  //    The timer has audio output that can announce the results of the race
  //    independently of this software.
  //    There are 5 modes:
  //    @ - No announcements
  //    A - Lanes in finish order only
  //    B - Lanes in finish order and winner time
  //    C - Lanes in finish order including all times
  //    D - Lanes in finish order including time differences
  //
  // Changing configuration.  Each configuration type uses a digit prefix
  //
  // Tiebreaker margin.
  //    Using the values above,
  //    To set configuration, prefix '1'
  //    1<char>\r
}
