package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

public interface TimerDevice {
    // Constructor takes SerialPortWrapper, but doesn't change settings.

    // Attempt to identify the device on this port.  Returns true if
    // device is recognized, false otherwise.  "Recognized" here
    // implies that this TimerDevice object knows how to manage this
    // timer.
    //
    // probe() should make any port setting changes necessary (baud rate, etc.).
    boolean probe() throws SerialPortException;

    // Returns 0 if can't tell/don't know
    int getNumberOfLanes() throws SerialPortException;

    public interface RaceFinishedCallback {
        // TODO: Need place information, not just times.
        void raceFinished(String[] results);
    }
    void registerRaceFinishedCallback(RaceFinishedCallback cb);

    public interface StartingGateCallback {
        void startGateChange(boolean isOpen);
    }
    void registerStartingGateCallback(StartingGateCallback cb);

    // Called when client is expecting a heat to be staged/started.
    void prepareHeat(int laneMask) throws SerialPortException;

    void abortHeat() throws SerialPortException;

    // Perform any recurring polling, mainly checking the starting
    // gate status.  Invoke callbacks as necessary.
    void poll() throws SerialPortException;
}
