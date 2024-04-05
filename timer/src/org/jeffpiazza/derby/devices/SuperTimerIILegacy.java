package org.jeffpiazza.derby.devices;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;

public class SuperTimerIILegacy extends TimerDeviceCommon {
  private TimerResult result = null;
  private int numberOfLanes = 4;  // Detected at probe time

  private static final String PROBE = "UUUUc\r";
  private static final String PROBE_RESPONSE = "UUUUc\r";
  private static final String QUERY_STATE = "00\r";
  private static final String CMD_TIMEOUT_10_SECONDS = "2h\r";
  private static final String CMD_TIEBREAKER_1MS = "1J\r";
  private static final String CMD_NO_ANNOUNCEMENTS = "4@\r";

  private static final Pattern laneNumberPattern = Pattern.compile("^#(\\d)$");
  private static final Pattern laneTimePattern = Pattern.compile("^(\\d+.\\d+)$");
  private static final Pattern raceFinishedPattern = Pattern.compile("^!$");

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
  // Timeout.
  //	To set configuration, prefix '2'
  //	2<char>\r
  // Lane Mask.
  //    To prepare for a race with a mask, prefix '3' and append "5A".
  //    3<char>5A\r
  //    To pause a race with a mask, prefix '3' and append "7A".
  //    3<char>7A\r
  // Announcement mode.
  //    To set configuration, prefix 4.
  //    4<char>\r
  //
  // DEFAULTS:
  // These are not configurable at runtime:
  //   Tiebreaker margin: 1ms
  //   Timeout: 10s, in keeping with other implementations
  //   Announcement Mode: No announcements
  //
  // The lane mask is configurable at runtime.

  public SuperTimerIILegacy(TimerPortWrapper portWrapper) {
    // No GateWatcher, because there's no way to poll this timer
    super(portWrapper, null, false);

    // The timer has a timeout itself, after which it reports all results
    // as the timeout value.  No software support beyond setting the timer
    // is required.
  }

  public static String toHumanString() {
    return "SuperTimer II (SuperTimer)";
  }

  @Override
  public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState)
			  throws SerialPortException {
    if (newState == RacingStateMachine.State.RESULTS_OVERDUE) {
      logOverdueResults();
    }
  }

  @Override
  public boolean canBeIdentified() {
    return true;
  }

  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE)) {
        LogWriter.serial("Probe failed to set up port");
      return false;
    }

    // Just forcing a new line, don't care about response.
    portWrapper.writeAndDrainResponse("\r");

    String confirm = portWrapper.writeAndWaitForResponse(PROBE);
    if (confirm == PROBE_RESPONSE) {
        LogWriter.serial("Probe failed with " + confirm);
	return false;
    }

    portWrapper.writeAndDrainResponse(QUERY_STATE);
    portWrapper.writeAndDrainResponse(CMD_TIMEOUT_10_SECONDS);
    portWrapper.writeAndDrainResponse(CMD_TIEBREAKER_1MS);
    portWrapper.writeAndDrainResponse(CMD_NO_ANNOUNCEMENTS);
    portWrapper.writeAndDrainResponse(QUERY_STATE);

    reset();
    setUp();

    return true;
  }

  private static int placeNumber;
  private static int currentLane;

  private void reset() {
    placeNumber = 0;
    currentLane = 0;
  }

  protected void setUp() {
    portWrapper.registerDetector(new TimerPortWrapper.Detector() {
      @Override
      public String apply(String line) throws SerialPortException {
	if (line.equals("!")) {
	  // end of race
	  Message.LaneResult[] resultArray = result.toArray();
	  result = null;
	  raceFinished(resultArray);
	  return "";
	} else {
	  Matcher m = laneNumberPattern.matcher(line);
	  if (m.find()) {
	    if (result != null) {
	      placeNumber++;
	      currentLane = Integer.parseInt(m.group(1));
	    }
	    return "";
	  }
	  m = laneTimePattern.matcher(line);
	  if (m.find()) {
	    if (result != null) {
	      float time = Float.parseFloat(m.group(1)) / 10000;

	      result.setLane(currentLane, String.valueOf(time), placeNumber);
	    }
	    return "";
	  }
	  return line;
	}
      }
    });
  }

  public int getNumberOfLanes() throws SerialPortException {
    return numberOfLanes;
  }

  private static final int MAX_LANES = 6;

  public int getSafeNumberOfLanes() {
    return numberOfLanes == 0 ? MAX_LANES : numberOfLanes;
  }

  private static String abortCmd;
  protected void maskLanes(int lanemask) throws SerialPortException {
    char laneChar = (char)(lanemask & ((1 << MAX_LANES) - 1));
    laneChar += '@';

    String cmd = "3" + laneChar + "5A\r";
    abortCmd = "3" + laneChar + "7A\r";

    // These don't give responses, so no need to wait for any.
    portWrapper.write(cmd);

    String confirm = portWrapper.writeAndWaitForResponse(PROBE);
    if (confirm == PROBE_RESPONSE) {
        LogWriter.serial("Probe failed with " + confirm);
    }

    result = new TimerResult(lanemask);
    reset();
  }

  public void abortHeat() throws SerialPortException {
    portWrapper.write(abortCmd);
    super.abortHeat();
    reset();
  }

  @Override
  protected void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException {

    if (state == RacingStateMachine.State.RESULTS_OVERDUE) {
      // FORCE_RACE_RESULTS was sent upon entering RESULTS_OVERDUE; see above.
      if (rsm.millisInCurrentState() > 1000) {
        giveUpOnOverdueResults();
      }
    }
  }
}
