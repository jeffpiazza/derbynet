package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

public class FastTrackDevice extends TimerDeviceBase implements TimerDevice {
    private boolean gateIsClosed;
    private SerialPortWrapper.Detector finishedDetector;

    public FastTrackDevice(SerialPortWrapper portWrapper) {
        super(portWrapper);
    }

    public static final int MAX_LANES = 6;

    private static final String PULSE_LASER_BIT = "LG";
    // LG actually starts the timer, but after a brief pause
    private static final String RESET_LASER_GATE = "LR";
    // LR appears to be the real "reset" command for the timer
    private static final String LANE_MASK = "M"; // + A to mask out lane 1, B lane 2, etc.
    private static final String CLEAR_LANE_MASK = LANE_MASK + "G";
    private static final String OLD_FORMAT = "N0"; //A=3.001! B=3.002 C=3.003 D=3.004 E=3.005 F=3.006 <LF> <CR>
    private static final String NEW_FORMAT = "N1"; //A=3.001! B=3.002” C=3.003# D=3.004$ E=3.005% F=3.006& <CR> <LF>
    private static final String ENHANCED_FORMAT = "N2";
    // N2 => 5-digit time and start switch open/cloed status, 2012 or newer timers only
    // private static final String COUNT_DOWN_TIMER = "PC"; // e.g., PC01 to count down one minute
    private static final String FORCE_RESULTS = "RA";
    // RA doesn't report anything unless at least one car has crossed the line
    private static final String RESET_ELIMINATOR_MODE = "RE";
    private static final String READ_START_SWITCH = "RG";
    private static final String REVERSE_LANES = "RL";  // + 0-6, number of lanes on track
    private static final String READ_MODE = "RM";
    private static final String READ_SERIAL_NUMBER = "RS";
    private static final String READ_VERSION = "RV";
    // private static final String FORCE_PRINT = "RX";  // requires "Force Print" option
    // RX resets the timer, but then seems to make it unresponsive

    // Some spurious "@" characters at front of result string, maybe?
    // A response of "X" indicates command not understood
    // E and F lanes get reported, regardless of masking
    //
    // GPRM test sequence is:
    // RS
    // 15985
    // RLO
    // *
    // RE
    // *
    // N2
    // X
    // N1
    // *

    // Reset timer sends LR
    // Test Race sends:
    // MG
    // ACLR
    // *
    // RG0
    public boolean probe() throws SerialPortException {
        if (!portWrapper.port().setParams(SerialPort.BAUDRATE_9600, SerialPort.DATABITS_8,
                                          SerialPort.STOPBITS_1, SerialPort.PARITY_NONE,
                                          /* rts */ false, /* dtr */ false)) {
            return false;
        }

        portWrapper.write(READ_VERSION);

        // We're looking for a response that matches these:
        // Copyright (c) Micro Wizard 2002-2005
        // K3 Version 1.05A  Serial Number 15985
        long deadline = System.currentTimeMillis() + 2000;
        String s;
        while ((s = portWrapper.next(deadline)) != null) {
            if (s.indexOf("Micro Wizard") >= 0) {
                System.out.println("* Micro Wizard detected");
                s = portWrapper.next(deadline);
                if (s.startsWith("K")) {
                    System.out.println("* K timer string detected");
                    gateIsClosed = isGateClosed();
                    return true;
                }
            }
        }

        return false;
    }

    public int getNumberOfLanes() throws SerialPortException {
        // TODO: Can we tell?  Don't think so.
        return 0;
    }

    public void registerRaceFinishedCallback(final RaceFinishedCallback raceFinishedCallback) {
        if (raceFinishedCallback == null) {
            setRaceFinishedCallback(null);
            setFinishedDetector(null);
        } else {
            SerialPortWrapper.Detector detector = new SerialPortWrapper.Detector() {
                    public boolean test(String line) {
                        Message.LaneResult[] results =
                            TimerDeviceUtils.parseCommonRaceResult(line, MAX_LANES);
                        if (results != null) {
                            raceFinishedCallback.raceFinished(results);
                            return true;
                        } else {
                            return false;
                        }
                    }
                };
            setRaceFinishedCallback(raceFinishedCallback);
            setFinishedDetector(detector);
            portWrapper.registerDetector(detector);
        }
    }

    public void prepareHeat(int lanemask) throws SerialPortException {
        portWrapper.writeAndWaitForResponse(RESET_LASER_GATE, 500);
        portWrapper.writeAndWaitForResponse(CLEAR_LANE_MASK, 500);
        for (int lane = 0; lane < MAX_LANES; ++lane) {
            if ((lanemask & (1 << lane)) == 0) {
                portWrapper.writeAndWaitForResponse(LANE_MASK + (char)('A' + lane), 500);
            }
        }
    }

    public void abortHeat() throws SerialPortException {
        // TODO
    }

    public void poll() throws SerialPortException {
        if (gateIsClosed != isGateClosed()) {
            gateIsClosed = !gateIsClosed;
            // TODO
            StartingGateCallback callback = getStartingGateCallback();
            if (callback != null) {
                callback.startGateChange(gateIsClosed);
            }
        }
    }

    private boolean isGateClosed() throws SerialPortException {
        // TODO: Huh? portWrapper.writeAndWaitForResponse(RESET_LASER_GATE, 500);
        portWrapper.write(READ_START_SWITCH);

        long deadline = System.currentTimeMillis() + 1000;
        String s;
        while ((s = portWrapper.next(deadline)) != null) {
            if (s.startsWith(READ_START_SWITCH)) {
                return s.charAt(2) == '1';
            }
        }

        // Don't know, assume unchanged
        portWrapper.log(SerialPortWrapper.INTERNAL, "Unable to determine starting gate state");
        return gateIsClosed;
    }
}
