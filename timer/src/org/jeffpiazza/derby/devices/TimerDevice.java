package org.jeffpiazza.derby.devices;

import jssc.*;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public interface TimerDevice {
  // Constructor takes SerialPortWrapper, but doesn't change settings.
  // probe() changes the port settings.

  public static class LostConnectionException extends Exception {
  }

  // Thrown to signify a query didn't receive a response in time, e.g.
  // interrogating starting gate state.
  public static class NoResponseException extends Exception {
  }

  // Callback to notify when the race actually starts (i.e., the gate
  // opens after a prepareHeat call).  This mostly supplants any real
  // use of StartingGateCallback.
  public interface RaceStartedCallback {
    void raceStarted();
  }

  // Callback to report the full heat result.  results is expected to contain
  // an entry for each non-masked lane in the heat.
  public interface RaceFinishedCallback {
    void raceFinished(int roundid, int heat, Message.LaneResult[] results);
  }

  // Callback invoked when a timer malfunction is detected, e.g., for lost
  // communication or unexpected error response from the timer.
  public interface TimerMalfunctionCallback {
    // detectable is true if a successful poll implies the problem has been resolved.
    // TODO Enhance poll() to indicate success/failure
    void malfunction(boolean detectable, String msg);
  }

  SerialPortWrapper getPortWrapper();

  // Returns true if the device can be prompted to identify itself somehow.
  // If false, probe(), below, will always return true, but without knowing
  // whether this is the correct device or not.
  boolean canBeIdentified();

  // Attempt to identify the device on this port.  Returns true if
  // device is recognized, false otherwise.  "Recognized" here
  // implies that this TimerDevice object knows how to manage this
  // timer.
  //
  // probe() should make any port setting changes necessary (baud rate, etc.).
  boolean probe() throws SerialPortException;

  // Returns 0 if can't tell/don't know
  int getNumberOfLanes() throws SerialPortException;
  String getTimerIdentifier();
  String humanName();

  void registerRaceStartedCallback(RaceStartedCallback cb);
  void registerRaceFinishedCallback(RaceFinishedCallback cb);
  void registerTimerMalfunctionCallback(TimerMalfunctionCallback cb);
  void invokeMalfunctionCallback(boolean detectable, String msg);

  // Called when client is expecting a heat to be staged/started.
  void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException;

  void abortHeat() throws SerialPortException;

  // Returns true if there's ever been any recognized data sent from the device.
  // There's reason to doubt the identity of a timer if it's never spoken.
  boolean hasEverSpoken();

  // Perform any recurring polling, mainly checking the starting
  // gate status.  Invoke callbacks as necessary.  Throws
  // LostConnectionException if the timer device becomes unresponsive for a
  // sufficiently long time.
  void poll() throws SerialPortException, LostConnectionException;

  void close();

  RemoteStartInterface getRemoteStart();
}
