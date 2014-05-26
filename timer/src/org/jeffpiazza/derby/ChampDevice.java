package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

public class ChampDevice extends TimerDeviceBase implements TimerDevice {
    private int numberOfLanes;  // Detected at probe time

    // These are all require synchronized access:
    private boolean gateIsClosed;
    private boolean racePending;
    private long raceFinishedDeadline;  // 0 for "not set"

    private static final String READ_DECIMAL_PLACES = "od\r";
    private static final String SET_DECIMAL_PLACES = "od";  // 3,4,5
    private static final String READ_LANE_CHARACTER = "ol\r";
    private static final String SET_LANE_CHARACTER_A = "ol0\r"; // report lane 1 as "A"
    // private static final String READ_LANE_MASK = "om\r";
    private static final String RESET_LANE_MASK = "om0\r";
    private static final String MASK_LANE = "om";  // + lane, e.g., om3
    private static final String READ_LANE_COUNT = "on\r";
    // private static final String SET_LANE_COUNT = "on"; // + count, eg on4
    private static final String READ_PLACE_CHARACTER = "op\r";
    private static final String SET_PLACE_CHARACTER_BANG = "op3\r";
    private static final String READ_AUTO_RESET = "or\r";
    private static final String RESET_REVERSE_LANES = "ov0\r";

    private static final String RESET = "r\r";
    private static final String FORCE_END_OF_RACE = "ra\r";
    private static final String READY_TIMER = "rg\r";  // "return results when race ends"
    private static final String READ_FINISH_LINE = "rl\r";  // bit mask, 0=inactive
    // private static final String RETURN_PREVIOUS = "rp\r";
    private static final String READ_RESET_SWITCH = "rr\r";
    private static final String READ_START_SWITCH = "rs\r";
    private static final String READ_VERSION = "v\r";

    public ChampDevice(SerialPortWrapper portWrapper) {
        super(portWrapper);
    }

    public boolean probe() throws SerialPortException {
        if (!portWrapper.port().setParams(SerialPort.BAUDRATE_9600, SerialPort.DATABITS_8,
                                          SerialPort.STOPBITS_1, SerialPort.PARITY_NONE,
                                          /* rts */ false, /* dtr */ false)) {
            return false;
        }

        portWrapper.writeAndWaitForResponse("\r");

        // Just forcing a new line, don't care about response.
        portWrapper.write(READ_VERSION);

        long deadline = System.currentTimeMillis() + 2000;
        String s;
        while ((s = portWrapper.next(deadline)) != null) {
            // eTekGadget SmartLine Timer v20.06 (B0007)
            if (s.indexOf("eTekGadget SmartLine Timer") >= 0) {

                portWrapper.write(RESET);

                String nl = portWrapper.writeAndWaitForResponse(READ_LANE_COUNT, 500);
                if ('0' < nl.charAt(0) && nl.charAt(0) <= '9') {
                    this.numberOfLanes = nl.charAt(0) - '0';
                }

                // TODO: Does this just need to be configured to
                // eliminate having to do manually?
                System.out.println("AUTO_RESET = " +  // TODO
                                   portWrapper.writeAndWaitForResponse(READ_AUTO_RESET, 500)
                                   ); 
                System.out.println("LANE_CHARACTER = " +
                                   portWrapper.writeAndWaitForResponse(READ_LANE_CHARACTER, 500)
                                   );
                System.out.println("DECIMAL_PLACES = " +
                                   portWrapper.writeAndWaitForResponse(READ_DECIMAL_PLACES, 500)
                                   );
                System.out.println("PLACE_CHARACTER = " +
                                   portWrapper.writeAndWaitForResponse(READ_PLACE_CHARACTER, 500)
                                   );
                System.out.println("START_SWITCH = " +
                                   portWrapper.writeAndWaitForResponse(READ_START_SWITCH, 500)
                                   );

                portWrapper.writeAndWaitForResponse(SET_LANE_CHARACTER_A, 500);
                portWrapper.writeAndWaitForResponse(SET_PLACE_CHARACTER_BANG, 500);

                return true;
            }
        }

        return false;
    }

    public int getNumberOfLanes() throws SerialPortException {
        return numberOfLanes;
    }

    private static final int MAX_LANES = 6;
    private int getSafeNumberOfLanes() {
        return numberOfLanes == 0 ? MAX_LANES : numberOfLanes;
    }

    public void registerRaceFinishedCallback(final RaceFinishedCallback raceFinishedCallback) {
        if (raceFinishedCallback == null) {
            setRaceFinishedCallback(null);
            setFinishedDetector(null);
        } else {
            SerialPortWrapper.Detector detector = new SerialPortWrapper.Detector() {
                    public boolean test(String line) {
                        Message.LaneResult[] results =
                            TimerDeviceUtils.parseCommonRaceResult(line, getSafeNumberOfLanes());
                        if (results != null) {
                            synchronized (ChampDevice.this) {
                                ChampDevice.this.racePending = false;
                                ChampDevice.this.raceFinishedDeadline = 0;
                            }
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

    protected synchronized long getRaceFinishedDeadline() { return raceFinishedDeadline; }
    protected synchronized void setRaceFinishedDeadline(long raceFinishedDeadline) {
        this.raceFinishedDeadline = raceFinishedDeadline;
    }

    protected synchronized boolean lastGateIsClosed() { return gateIsClosed; }

    public void prepareHeat(int lanemask) throws SerialPortException {
        // These don't give responses, so no need to wait for any.
        portWrapper.write(RESET_LANE_MASK);

        for (int lane = 0; lane < getSafeNumberOfLanes(); ++lane) {
            if ((lanemask & (1 << lane)) == 0) {
                portWrapper.write(MASK_LANE + (char) ('1' + lane) + "\r");
            }
        }

        // TODO: 
        synchronized (this) {
            // When expecting results, once the gate opens, we'll
            // force race results after a certain amount of time.
            this.racePending = true;
        }
    }

    public void abortHeat() throws SerialPortException {  // TODO?
    }

    // States:
    // - nothing.
    // - racePending set: prepareHeat called; lanes masked.
    // - wait for gate closed
    // - waiting for results: gate detected open; rg sent; raceFinishedDeadline set
    private boolean waitingForGateToOpen = false;
    
    public void poll() throws SerialPortException {
        boolean rp = false;
        long deadline = 0;
        synchronized (this) {
            rp = this.racePending;
            deadline = this.raceFinishedDeadline;
        }

        if (deadline != 0) {  // Waiting for results
            if (System.currentTimeMillis() >= deadline) {
                System.out.println("Forcing end of race");  // TODO
                setRaceFinishedDeadline(0);
                portWrapper.write(FORCE_END_OF_RACE);
            }
        } else {
            if (lastGateIsClosed() != interrogateStartingGateClosed()) {
                StartingGateCallback callback = null;
                boolean closed = false;
                synchronized (this) {
                    closed = !this.gateIsClosed;
                    this.gateIsClosed = closed;
                    callback = getStartingGateCallback();
                }

                if (waitingForGateToOpen) {
                    if (!closed) {
                        portWrapper.write(READY_TIMER);
                        synchronized (this) {
                            this.raceFinishedDeadline = System.currentTimeMillis() + 10000;
                            System.out.println("Gate opened; Race deadline set at " + raceFinishedDeadline);  // TODO
                            waitingForGateToOpen = false;
                        }
                    }
                } else if (rp) {
                    if (closed) {
                        System.out.println("Gate closed; waiting for race to start");  // TODO
                        this.waitingForGateToOpen = true;
                    }
                }

                if (callback != null) {
                    callback.startGateChange(!lastGateIsClosed());
                }

            }
        }
    }

    private boolean interrogateStartingGateClosed() throws SerialPortException {
        portWrapper.write(READ_START_SWITCH);
        long deadline = System.currentTimeMillis() + 500;
        String s;
        while ((s = portWrapper.next(deadline)) != null) {
            if (s.equals("1")) {
                return false;
            } else if (s.equals("0")) {
                return true;
            }
        }

        // Don't know, assume unchanged
        portWrapper.log(SerialPortWrapper.INTERNAL, "Unable to determine starting gate state");
        return lastGateIsClosed();
    }


}
