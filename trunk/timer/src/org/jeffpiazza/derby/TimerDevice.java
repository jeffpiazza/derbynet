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
    void raceFinished(Message.LaneResult[] results);
  }
  void registerRaceFinishedCallback(RaceFinishedCallback cb);

  // Callback to notify when the race actually starts (i.e., the gate
  // opens after a prepareHeat call).  This mostly supplants any real
  // use of StartingGateCallback.
  public interface RaceStartedCallback {
    void raceStarted();
  }
  void registerRaceStartedCallback(RaceStartedCallback cb);

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
