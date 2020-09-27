package org.jeffpiazza.derby;

import java.io.File;
import jssc.*;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.gui.TimerGui;

import javax.swing.*;
import org.jeffpiazza.derby.devices.AllDeviceTypes;
import org.jeffpiazza.derby.devices.RemoteStartInterface;
import org.jeffpiazza.derby.devices.TimerTask;

// Three threads for three "actors":
// timer polling loop runs on main thread,
// HttpTask runs on another thread,
// GUI event dispatch runs on a third thread.
public class TimerMain {
  public static void usage() {
    System.err.println("Usage: [options] <base-url>");
    Flag.usage();

    System.err.println("      Known devices:");
    AllDeviceTypes.listDeviceClassNames();
    System.err.println();
  }

  private static void makeLogWriter() {
    try {
      LogWriter.initialize();
      LogWriter.info("derby-timer.jar version " + Version.get());
      LogWriter.info("os.name = " + System.getProperty("os.name"));
    } catch (Throwable t) {
      t.printStackTrace();
      System.exit(1);
    }
  }

  public static void main(String[] args) {
    int consumed_args = Flag.parseCommandLineFlags(args, 0);

    if (Flag.version.value()) {
      System.out.println("DerbyNet version " + Version.get());
      System.exit(0);
    }

    String base_url = null;
    if (consumed_args < args.length && args[consumed_args].charAt(0) != '-') {
      base_url = args[consumed_args];
      ++consumed_args;
    }

    if (consumed_args < args.length) {
      usage();
      System.exit(1);
    }

    if (Flag.headless.value()) {
      if (base_url == null && !Flag.simulate_host.value()) {
        usage();
        System.exit(1);
      }
    }

    makeLogWriter();
    if (args.length > 0) {
      LogWriter.info("===== Command Line Arguments ==========");
      for (String arg : args) {
        LogWriter.info(arg);
      }
      LogWriter.info("=======================================");
    }

    ConnectorImpl connector = new ConnectorImpl(Flag.trace_messages.value());

    SimulatedClientSession simulatedSession
        = Flag.simulate_host.value() ? new SimulatedClientSession() : null;
    try {
      TimerGui timerGui = null;
      if (!Flag.headless.value()) {
        timerGui = startTimerGui(connector, base_url, simulatedSession);
      } else {
        final ClientSession clientSession
            = simulatedSession == null ? new ClientSession(base_url)
              : simulatedSession;
        LogWriter.setClientSession(clientSession);
        HttpTask.start(clientSession, connector,
                       new HttpTask.LoginCallback() {
                     @Override
                     public void onLoginSuccess() {
                       System.err.println("Successful login");
                     }

                     @Override
                     public void onLoginFailed(String message) {
                       System.err.println("Unsuccessful login: " + message);
                       System.exit(1);
                     }
                   });
      }

      TimerTask timerTask = new TimerTask(Flag.portname.value(),
                                          Flag.devicename.value(), timerGui,
                                          connector);
      if (Flag.simulate_timer.value()) {
        timerTask.setSimulatedTimer();
      }
      if (Flag.record.value()) {
        timerTask.setRecording();
      }
      if (Flag.playback.value() != null) {
        timerTask.setPlayback();
      }
      timerTask.run();
    } catch (Throwable t) {
      LogWriter.stacktrace(t);
    }
  }

  private static TimerGui startTimerGui(ConnectorImpl connector,
                                        String base_url,
                                        ClientSession simulatedSession) {
    final TimerGui timerGui = new TimerGui(connector);
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        timerGui.show();
      }
    });
    if (base_url != null) {
      timerGui.setUrl(base_url);
    }
    timerGui.setRoleAndPassword(Flag.username.value(),
                                Flag.password.value());
    if (simulatedSession != null) {
      timerGui.setClientSession(simulatedSession);
    }
    return timerGui;
  }

  // Allow the timer device and web server connection to come up in either
  // order, or perhaps not at all; when they're both established, wire together
  // callbacks and send hello with lane count to web server.
  public static class ConnectorImpl implements Connector {
    private HttpTask httpTask;
    private TimerTask timerTask;
    private boolean traceMessages;

    public ConnectorImpl(boolean traceMessages) {
      this.traceMessages = traceMessages;
    }

    @Override
    public synchronized void setHttpTask(HttpTask httpTask) {
      this.httpTask = httpTask;
      maybeWireTogether();
    }

    @Override
    public synchronized void setTimerTask(TimerTask deviceTask) {
      this.timerTask = deviceTask;
      maybeWireTogether();
    }

    private void maybeWireTogether() {
      if (httpTask != null && timerTask != null && timerTask.device() != null) {
        wireTogether(httpTask, timerTask, traceMessages);
        int nlanes = 0;
        try {
          nlanes = timerTask.device().getNumberOfLanes();
        } catch (SerialPortException e) {
          LogWriter.stacktrace(e);
        }
        httpTask.sendIdentified(
            nlanes, timerTask.device().getClass().getSimpleName(),
            timerTask.device().getTimerIdentifier(),
            timerTask.device().hasEverSpoken());
      }
    }

    // Registers callbacks that allow the httpTask and timer device to
    // communicate asynchronously.
    public static void wireTogether(final HttpTask httpTask,
                                    final TimerTask timerTask,
                                    final boolean traceMessages) {
      if (traceMessages) {
        StdoutMessageTrace.trace("Timer detected.");
      }

      httpTask.registerTimerHealthCallback(timerTask);

      httpTask.registerHeatReadyCallback(new HttpTask.HeatReadyCallback() {
        public void onHeatReady(int roundid, int heat, int laneMask) {
          try {
            if (traceMessages) {
              StdoutMessageTrace.trace(
                  "Heat ready: roundid=" + roundid + ", heat=" + heat);
            }
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
          if (traceMessages) {
            StdoutMessageTrace.trace("AbortHeat received");
          }
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

      timerTask.device().registerRaceStartedCallback(
          new TimerDevice.RaceStartedCallback() {
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

      timerTask.device().registerRaceFinishedCallback(
          new TimerDevice.RaceFinishedCallback() {
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
            httpTask.queueMessage(new Message.Finished(roundid, heat, results));
          } catch (Throwable t) {
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
}
