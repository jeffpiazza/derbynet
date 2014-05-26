package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;

public abstract class TimerDeviceBase implements TimerDevice {
    protected SerialPortWrapper portWrapper;

    private RaceFinishedCallback raceFinishedCallback;
    private StartingGateCallback startingGateCallback;
    private SerialPortWrapper.Detector finishedDetector;

    TimerDeviceBase(SerialPortWrapper portWrapper) {
        this.portWrapper = portWrapper;
    }

    protected synchronized RaceFinishedCallback getRaceFinishedCallback() { return raceFinishedCallback; }
    protected synchronized void setRaceFinishedCallback(RaceFinishedCallback raceFinishedCallback) {
        this.raceFinishedCallback = raceFinishedCallback;
    }

    protected synchronized StartingGateCallback getStartingGateCallback() { return startingGateCallback; }
    public synchronized void registerStartingGateCallback(StartingGateCallback startingGateCallback) {
        this.startingGateCallback = startingGateCallback;
    }

    protected synchronized void setFinishedDetector(SerialPortWrapper.Detector finishedDetector) {
        if (this.finishedDetector != null) {
            portWrapper.unregisterDetector(this.finishedDetector);
        }
        this.finishedDetector = finishedDetector;
    }
}
