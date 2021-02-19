package org.jeffpiazza.derby;

import java.io.File;
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

  public void setTimerGui(TimerGui gui) {
    this.timerGui = gui;
  }

  public synchronized void setHttpTask(HttpTask httpTask) {
    this.httpTask = httpTask;
    maybeWireTogether();
  }

  public synchronized void setTimerTask(TimerTask deviceTask) {
    this.timerTask = deviceTask;
    maybeWireTogether();
  }

  private void maybeWireTogether() {
    if (httpTask == null || timerTask == null) {
      return;
    }
    prewire(httpTask, timerTask, timerGui);
    TimerDevice device = timerTask.device();
    if (device != null) {
      wireTogether(httpTask, timerTask);
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
        if (timerGui != null) {
          timerGui.setSerialPort(portName);
        }
        timerTask.userChoosesSerialPort(portName);
        LogWriter.info("Assigned port " + portName);
      }
    });
    httpTask.registerAssignDeviceCallback(new HttpTask.AssignDeviceCallback() {
      @Override
      public void onAssignDevice(String deviceName) {
        Class<? extends TimerDevice> cl = AllDeviceTypes.getDeviceClass(
            deviceName);
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
  private static void wireTogether(final HttpTask httpTask,
                                   final TimerTask timerTask) {
    StdoutMessageTrace.trace("Timer-Server connection established.");
    httpTask.registerHeatReadyCallback(new HttpTask.HeatReadyCallback() {
      public void onHeatReady(int roundid, int heat, int laneMask) {
        try {
          timerTask.device().prepareHeat(roundid, heat, laneMask);
        } catch (Throwable t) {
          LogWriter.stacktrace(t);
          httpTask.queueMessage(new Message.Malfunction(false,
                                                        "Can't ready timer."));
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
        if (timerTask.device() instanceof RemoteStartInterface) {
          RemoteStartInterface rs = (RemoteStartInterface) timerTask.device();
          return rs.hasRemoteStart();
        }
        return false;
      }

      @Override
      public void remoteStart() {
        if (timerTask.device() instanceof RemoteStartInterface) {
          RemoteStartInterface rs = (RemoteStartInterface) timerTask.device();
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
    timerTask.device().
        registerRaceStartedCallback(new TimerDevice.RaceStartedCallback() {
          public void raceStarted() {
            if (Flag.trigger_file_directory.value != null) {
              try {
                (new File(new File(Flag.trigger_file_directory.value),
                          "heat-started")).createNewFile();
              } catch (Throwable t) {
                LogWriter.info("Failed to create /tmp/heat-started: " + t.
                    getMessage());
              }
            }
            try {
              httpTask.queueMessage(new Message.Started());
            } catch (Throwable t) {
            }
          }
        });
    timerTask.device().
        registerRaceFinishedCallback(new TimerDevice.RaceFinishedCallback() {
          public void raceFinished(int roundid, int heat,
                                   Message.LaneResult[] results) {
            if (Flag.trigger_file_directory.value != null) {
              try {
                (new File(new File(Flag.trigger_file_directory.value),
                          "heat-finished")).createNewFile();
              } catch (Throwable t) {
                LogWriter.info("Failed to create /tmp/heat-finished: " + t.
                    getMessage());
              }
            }
            // Rely on recipient to ignore if not expecting any results
            try {
              httpTask.
                  queueMessage(new Message.Finished(roundid, heat, results));
            } catch (Throwable t) {
            }
          }
        });
    timerTask.device().
        registerTimerMalfunctionCallback(
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
