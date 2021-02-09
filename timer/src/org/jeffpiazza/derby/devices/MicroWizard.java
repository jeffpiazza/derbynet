package org.jeffpiazza.derby.devices;

// Some common functionality shared between FastTrackDevice and
// OlderFastTrackDevice classes.
import jssc.SerialPortException;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public class MicroWizard {
  public static final String PULSE_LASER_BIT = "LG";
  // LG actually starts the timer, but after a brief pause.  More importantly,
  // it triggers the automatic gate release, if installed.
  public static final String RESET_LASER_GATE = "LR";
  // LR appears to be the real "reset" command for the timer
  public static final String LANE_MASK = "M"; // + A to mask out lane 1, B lane 2, etc.
  public static final String CLEAR_LANE_MASK = LANE_MASK + "G";
  public static final String OLD_FORMAT = "N0"; //A=3.001! B=3.002 C=3.003 D=3.004 E=3.005 F=3.006 <LF> <CR>
  public static final String NEW_FORMAT = "N1"; //A=3.001! B=3.002" C=3.003# D=3.004$ E=3.005% F=3.006& <CR> <LF>
  public static final String ENHANCED_FORMAT = "N2";
  // N2 => 5-digit time and start switch open/closed status, 2012 or newer timers only
  // public static final String COUNT_DOWN_TIMER = "PC"; // e.g., PC01 to count down one minute
  public static final String FORCE_RESULTS = "RA";
  // RA doesn't report anything unless at least one car has crossed the line
  // But it will stop the timer...
  public static final String RESET_ELIMINATOR_MODE = "RE";
  public static final String READ_START_SWITCH = "RG";
  public static final String REVERSE_LANES = "RL";  // + 0-6, number of lanes on track
  public static final String READ_MODE = "RM";
  public static final String READ_SERIAL_NUMBER = "RS";
  public static final String READ_VERSION = "RV";

  // Not sure this even works, or how to interpret the response, but it may
  // be (from K3 instructions):
  //
  // S - Serial Race Data
  // M - Mask Lanes
  // R - Reverse Lanes
  // E - Eliminator
  // F - Force Print
  // L - Reset Laser
  // C - Count Down Clock
  // P - Position (sequence of finish)

  // A K1 timer from 2004, version 1.09D, reported 0011 1111, and
  // reported "X" (disabled) for reading the start switch ("RG")
  public static final String RETURN_FEATURES = "RF";

  public static void readFeatures(SerialPortWrapper portWrapper)
      throws SerialPortException {
    if (!Flag.skip_read_features.value()) {
      // Capture features to the log, for diagnostic purposes
      portWrapper.writeAndDrainResponse(RETURN_FEATURES, 2, 1000);
    }
  }

  // public static final String FORCE_PRINT = "RX";  // requires "Force Print" option
  // RX resets the timer, but then seems to make it unresponsive
  public static void registerEarlyDetectorForReset(SerialPortWrapper portWrapper) {
    portWrapper.registerEarlyDetector(new SerialPortWrapper.Detector() {
      @Override
      public String apply(String s) throws SerialPortException {
        while (!s.isEmpty() && (s.charAt(0) == '@' || s.charAt(0) == '>')) {
          s = s.substring(1);
        }
        return s;
      }
    });
  }
}
