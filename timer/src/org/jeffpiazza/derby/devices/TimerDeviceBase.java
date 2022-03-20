package org.jeffpiazza.derby.devices;

import java.util.logging.Level;
import java.util.logging.Logger;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public abstract class TimerDeviceBase implements TimerDevice {
  protected SerialPortWrapper portWrapper;

  private RaceStartedCallback raceStartedCallback;
  private RaceFinishedCallback raceFinishedCallback;
  private TimerMalfunctionCallback timerMalfunctionCallback;

  // Identifiers for the currently-prepared heat
  protected int roundid;
  protected int heat;

  protected TimerDeviceBase(SerialPortWrapper portWrapper) {
    this.portWrapper = portWrapper;
    this.roundid = this.heat = 0;
  }

  @Override
  public String humanName() {
    return AllDeviceTypes.toHumanString(this.getClass());
  }

  public SerialPortWrapper getPortWrapper() {
    return portWrapper;
  }

  @Override
  public boolean canBeIdentified() {
    return true;  // by default
  }

  @Override
  public boolean hasEverSpoken() {
    return portWrapper.hasEverSpoken();
  }

  protected void prepare(int roundid, int heat) {
    this.roundid = roundid;
    this.heat = heat;
  }

  // Checks if we've had any response recently; if not, invokes
  // malfunction callback.
  protected void checkConnection() throws LostConnectionException {
    if (portWrapper.millisSinceLastContact() > 2000) {
      throw new LostConnectionException();
    }
  }

  public synchronized void registerRaceStartedCallback(
      RaceStartedCallback raceStartedCallback) {
    this.raceStartedCallback = raceStartedCallback;
  }

  protected synchronized RaceStartedCallback getRaceStartedCallback() {
    return raceStartedCallback;
  }

  protected void invokeRaceStartedCallback() {
    RaceStartedCallback cb = getRaceStartedCallback();
    if (cb != null) {
      cb.raceStarted();
    }
  }

  public synchronized void registerRaceFinishedCallback(
      RaceFinishedCallback raceFinishedCallback) {
    this.raceFinishedCallback = raceFinishedCallback;
  }

  protected synchronized RaceFinishedCallback getRaceFinishedCallback() {
    return raceFinishedCallback;
  }

  protected void invokeRaceFinishedCallback(int roundid, int heat, Message.LaneResult[] results) {
    RaceFinishedCallback cb = getRaceFinishedCallback();
    if (cb != null) {
      cb.raceFinished(roundid, heat, results);
    }
  }

  public synchronized void registerTimerMalfunctionCallback(
      TimerMalfunctionCallback timerMalfunctionCallback) {
    this.timerMalfunctionCallback = timerMalfunctionCallback;
  }

  protected synchronized TimerMalfunctionCallback getTimerMalfunctionCallback() {
    return timerMalfunctionCallback;
  }

  @Override
  public synchronized void invokeMalfunctionCallback(boolean detectable,
                                                     String msg) {
    TimerMalfunctionCallback cb = getTimerMalfunctionCallback();
    if (cb != null) {
      cb.malfunction(detectable, msg);
    }
  }

  @Override
  public void close() {
    try {
      portWrapper.closePort();
    } catch (SerialPortException ex) {
      Logger.getLogger(TimerDeviceBase.class.getName())
          .log(Level.SEVERE, null, ex);
    }
  }

  @Override
  public RemoteStartInterface getRemoteStart() {
    return null;
  }
}
