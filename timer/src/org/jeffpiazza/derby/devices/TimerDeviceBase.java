package org.jeffpiazza.derby.devices;

import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.SerialPortWrapper;

public abstract class TimerDeviceBase implements TimerDevice {
  protected SerialPortWrapper portWrapper;

  private RaceStartedCallback raceStartedCallback;
  private RaceFinishedCallback raceFinishedCallback;
  private StartingGateCallback startingGateCallback;
  private TimerMalfunctionCallback timerMalfunctionCallback;

  protected TimerDeviceBase(SerialPortWrapper portWrapper) {
    this.portWrapper = portWrapper;
  }

  // Returns true if we've had any response recently, otherwise invokes
  // malfunction callback and returns false.
  protected boolean checkConnection() {
    if (portWrapper.millisSinceLastContact() < 2000) {
      return true;
    } else {
      String msg = "No response from timer in " + portWrapper.millisSinceLastContact() + "ms.";
      portWrapper.logWriter().serialPortLogInternal(msg);
      malfunction(true, msg);
      return false;
    }      
  }


  public synchronized void registerRaceStartedCallback(RaceStartedCallback raceStartedCallback) {
    this.raceStartedCallback = raceStartedCallback;
  }
  protected synchronized RaceStartedCallback getRaceStartedCallback() {
    return raceStartedCallback;
  }
  protected void raceStarted() {
    RaceStartedCallback cb = getRaceStartedCallback();
    if (cb != null) {
      cb.raceStarted();
    }
  }

  public synchronized void registerRaceFinishedCallback(RaceFinishedCallback raceFinishedCallback) {
    this.raceFinishedCallback = raceFinishedCallback;
  }
  protected synchronized RaceFinishedCallback getRaceFinishedCallback() {
    return raceFinishedCallback;
  }
  protected void raceFinished(Message.LaneResult[] results) {
    RaceFinishedCallback cb = getRaceFinishedCallback();
    if (cb != null) {
      cb.raceFinished(results);
    }
  }

  public synchronized void registerStartingGateCallback(StartingGateCallback startingGateCallback) {
    this.startingGateCallback = startingGateCallback;
  }
  protected synchronized StartingGateCallback getStartingGateCallback() {
    return startingGateCallback;
  }
  protected void startGateChange(boolean isOpen) {
    StartingGateCallback cb = getStartingGateCallback();
    if (cb != null) {
      cb.startGateChange(isOpen);
    }
  }

  public synchronized void registerTimerMalfunctionCallback(TimerMalfunctionCallback timerMalfunctionCallback) {
    this.timerMalfunctionCallback = timerMalfunctionCallback;
  }
  protected synchronized TimerMalfunctionCallback getTimerMalfunctionCallback() {
    return timerMalfunctionCallback;
  }
  protected void malfunction(boolean detectable, String msg) {
    TimerMalfunctionCallback cb = getTimerMalfunctionCallback();
    if (cb != null) {
      cb.malfunction(detectable, msg);
    }
  }
}
