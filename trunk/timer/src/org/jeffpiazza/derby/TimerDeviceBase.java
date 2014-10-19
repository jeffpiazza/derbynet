package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;

public abstract class TimerDeviceBase implements TimerDevice {
  protected SerialPortWrapper portWrapper;

  private RaceStartedCallback raceStartedCallback;
  private RaceFinishedCallback raceFinishedCallback;
  private StartingGateCallback startingGateCallback;

  TimerDeviceBase(SerialPortWrapper portWrapper) {
    this.portWrapper = portWrapper;
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
}
