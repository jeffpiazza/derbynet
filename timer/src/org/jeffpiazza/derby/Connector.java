package org.jeffpiazza.derby;

import java.util.ArrayList;
import jssc.SerialPortException;
import org.jeffpiazza.derby.devices.AllDeviceTypes;
import org.jeffpiazza.derby.devices.RemoteStartInterface;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.devices.TimerTask;
import org.jeffpiazza.derby.gui.TimerGui;

// Allow the timer device and web server connection to come up in either
// order, or perhaps not at all; when they're both established, wire together
// callbacks and send hello with lane count to web server.
public class Connector {
  private HttpTask httpTask;
  private TimerTask timerTask;
  private TimerGui timerGui;
  private ArrayList<TimerDevice.RaceStartedCallback> raceStartedCallbacks;
  private ArrayList<TimerDevice.RaceFinishedCallback> raceFinishedCallbacks;

  public void setTimerGui(TimerGui gui) {
    this.timerGui = gui;
  }

  public synchronized void setHttpTask(HttpTask httpTask) {
    HttpTask originalHttpTask = this.httpTask;
    this.httpTask = httpTask;
    maybeWireTogether();
    if (originalHttpTask != null) {
      originalHttpTask.setShouldExit();
    }
  }

  public synchronized void setTimerTask(TimerTask deviceTask) {
    this.timerTask = deviceTask;
    maybeWireTogether();
  }

  public synchronized void addRaceStartedCallback(
      TimerDevice.RaceStartedCallback callback) {
    if (raceStartedCallbacks == null) {
      raceStartedCallbacks = new ArrayList<TimerDevice.RaceStartedCallback>();
    }
    raceStartedCallbacks.add(callback);
  }

  public synchronized void addRaceFinishedCallback(
      TimerDevice.RaceFinishedCallback callback) {
    if (raceFinishedCallbacks == null) {
      raceFinishedCallbacks = new ArrayList<TimerDevice.RaceFinishedCallback>();
    }
    raceFinishedCallbacks.add(callback);
  }

  private void maybeWireTogether() {
    if (httpTask == null || timerTask == null) {
      return;
    }
    prewire(httpTask, timerTask, timerGui);
    TimerDevice device = timerTask.device();
    if (device != null) {
      wireTogether(httpTask, timerTask,
                   raceStartedCallbacks, raceFinishedCallbacks);
      int nlanes = 0;
      try {
        nlanes = device.getNumberOfLanes();
      } catch (SerialPortException e) {
        LogWriter.stacktrace(e);
      }
      httpTask.sendIdentified(nlanes, device.getClass().getSimpleName(),
                              device.humanName(), device.getTimerIdentifier(),
                              device.hasEverSpoken());
    }
  }

  public static void prewire(final HttpTask httpTask, final TimerTask timerTask,
                             final TimerGui timerGui) {
    httpTask.registerTimerHealthCallback(timerTask);
    httpTask.registerAssignPortCallback(new HttpTask.AssignPortCallback() {
      @Override
      public void onAssignPort(String portName) {
        timerTask.userChoosesSerialPort(portName);
        LogWriter.info("Assigned port " + portName);
      }
    });
    httpTask.registerAssignDeviceCallback(new HttpTask.AssignDeviceCallback() {
      @Override
      public void onAssignDevice(String deviceName) {
        Class<? extends TimerDevice> cl
            = AllDeviceTypes.getDeviceClass(deviceName);
        if (timerGui != null) {
          timerGui.setTimerClass(cl);
        }
        timerTask.userChoosesTimerClass(cl);
        LogWriter.info("Assigned device " + deviceName);
      }
    });
  }

  // Registers callbacks that allow the httpTask and timer device to
  // communicate asynchronously.
  private static void wireTogether(
      final HttpTask httpTask,
      final TimerTask timerTask,
      final ArrayList<TimerDevice.RaceStartedCallback> raceStartedCallbacks,
      final ArrayList<TimerDevice.RaceFinishedCallback> raceFinishedCallbacks) {
    StdoutMessageTrace.trace("Timer-Server connection established.");
    httpTask.registerHeatReadyCallback(new HttpTask.HeatReadyCallback() {
      public void onHeatReady(int roundid, int heat, int laneMask) {
        if (timerTask.device() == null) {
          return;
        }
        try {
          timerTask.device().prepareHeat(roundid, heat, laneMask);
        } catch (Throwable t) {
          LogWriter.stacktrace(t);
          httpTask.queueMessage(
              new Message.Malfunction(false, "Can't ready timer."));
        }
      }
    });
    httpTask.registerAbortHeatCallback(new HttpTask.AbortHeatCallback() {
      public void onAbortHeat() {
        try {
          timerTask.device().abortHeat();
        } catch (Throwable t) {
          LogWriter.stacktrace(t);
          t.printStackTrace();
        }
      }
    });
    httpTask.registerRemoteStart(new HttpTask.RemoteStartCallback() {
      @Override
      public boolean hasRemoteStart() {
        if (timerTask.device() == null) {
          return false;
        }
        RemoteStartInterface rs = timerTask.device().getRemoteStart();
        return rs != null && rs.hasRemoteStart();
      }

      @Override
      public void remoteStart() {
        RemoteStartInterface rs = timerTask.device().getRemoteStart();
        if (rs != null) {
          try {
            LogWriter.serial("Executing remote-start");
            rs.remoteStart();
          } catch (SerialPortException ex) {
            LogWriter.stacktrace(ex);
          }
        } else {
          LogWriter.serial("Unable to respond to remote-start");
        }
      }
    });
    timerTask.device().registerRaceStartedCallback(
        new TimerDevice.RaceStartedCallback() {
      public void raceStarted() {
        try {
          httpTask.queueMessage(new Message.Started());
        } catch (Throwable t) {
        }
        if (raceStartedCallbacks != null) {
          for (TimerDevice.RaceStartedCallback cb : raceStartedCallbacks) {
            cb.raceStarted();
          }
        }
      }
    });
    timerTask.device().registerRaceFinishedCallback(
        new TimerDevice.RaceFinishedCallback() {
      public void raceFinished(int roundid, int heat,
                               Message.LaneResult[] results) {
        // Rely on recipient to ignore if not expecting any results
        try {
          httpTask.queueMessage(new Message.Finished(roundid, heat, results));
        } catch (Throwable t) {
        }
        if (raceFinishedCallbacks != null) {
          for (TimerDevice.RaceFinishedCallback cb : raceFinishedCallbacks) {
            cb.raceFinished(roundid, heat, results);
          }
        }
      }
    });
    timerTask.device().registerTimerMalfunctionCallback(
        new TimerDevice.TimerMalfunctionCallback() {
      public void malfunction(boolean detectable, String msg) {
        try {
          httpTask.queueMessage(new Message.Malfunction(detectable, msg));
        } catch (Throwable t) {
        }
      }
    });
  }
}
