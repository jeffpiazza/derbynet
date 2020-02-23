package org.jeffpiazza.derby;

import jssc.*;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.gui.TimerGui;

import javax.swing.*;
import org.jeffpiazza.derby.devices.AllDeviceTypes;
import org.jeffpiazza.derby.devices.FastTrackDevice;
import org.jeffpiazza.derby.devices.MicroWizard;
import org.jeffpiazza.derby.devices.NewBoldDevice;
import org.jeffpiazza.derby.devices.SimulatedDevice;
import org.jeffpiazza.derby.devices.TheJudgeDevice;
import org.jeffpiazza.derby.devices.TimerDeviceCommon;
import org.jeffpiazza.derby.devices.TimerTask;
import org.jeffpiazza.derby.serialport.PlaybackSerialPortWrapper;

// Three threads for three "actors":
// timer polling loop runs on main thread,
// HttpTask runs on another thread,
// GUI event dispatch runs on a third thread.
public class TimerMain {
  public static void usage() {
    System.err.println("Usage: [options] <base-url>");
    System.err.println("   -h or -help or --help: This message");
    System.err.println("   -v: Show version");
    System.err.println("   -x: Run headless, without gui.");
    System.err.println("   -logdir <directory>: write log files in <directory>");
    System.err.println("       instead of the current directory.");
    System.err.println("   -t: Trace non-heartbeat messages sent");
    System.err.println("   -th: Trace heartbeat messages sent");
    System.err.println("   -r: Show responses to traced messages");
    System.err.println(
        "   -u <user>: Specify username for authenticating to web server");
    System.err.println(
        "   -p <password>: Specify password for authenticating to web server");
    System.err.println(
        "   -n <port name>: Use specified port name instead of searching");
    System.err.println(
        "   -min-gate-time <milliseconds>: Ignore gate transitions shorter than <milliseconds>");
    System.err.println(
        "  -ignore-place: Discard any place indications from timer");
    System.err.println(
        "   -d <device name>: Use specified device instead of trying to identify");
    System.err.println("      Known devices:");
    AllDeviceTypes.listDeviceClassNames();
    System.err.println(
        "   -delay-reset-after-race <nsec>: how long after race over");
    System.err.println(
        "                            before timer will be reset, default 10,");
    System.err.println(
        "                            for SmartLine, DerbyMagic, NewBold, and BertDrake");
    System.err.
        println("   -simulate-timer: Simulate timer device (for testing)");
    System.err.println("     -lanes <n>: Specify number of lanes to report");
    System.err.
        println("     -pace <nsec>: Staging pace (seconds between heats)");
    System.err.println("  -simulate-host: Exercise timer with simulated host");
    System.err.
        println("     -lanes <n>: Specify number of lanes for scheduling");
    System.err.println();
    System.err.println("Experimental flags for The Judge only:");
    System.err.println(
        "   -reset-on-ready: reset timer when next heat scheduled");
    System.err.println(
        "   -reset-on-race-over: reset timer immediately after Race Over from timer");
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
    String username = null;
    String password = null;
    String portname = null;
    String devicename = null;
    boolean showGui = true;
    boolean simulateTimer = false;
    boolean simulateHost = false;
    boolean recording = false;
    boolean playback = false;

    // Include HTTP traffic in the timer log:
    boolean traceMessages = false;
    boolean traceHeartbeats = false;
    boolean traceResponses = false;

    int consumed_args = 0;
    while (consumed_args < args.length && args[consumed_args].startsWith("-")) {
      final String arg = args[consumed_args];
      final boolean has_value = (args.length - consumed_args) > 1;
      if (arg.equals("-v")) {
        System.out.println("DerbyNet version " + Version.get());
        System.exit(0);
      } else if (arg.equals("-x")) {
        showGui = false;
        ++consumed_args;
      } else if (arg.equals("-logdir") && has_value) {
        LogWriter.setLogFileDirectory(args[consumed_args + 1]);
        consumed_args += 2;
      } else if (arg.equals("-t")) {
        traceMessages = true;
        ++consumed_args;
      } else if (arg.equals("-th")) {
        traceHeartbeats = true;
        ++consumed_args;
      } else if (arg.equals("-r")) { // Won't have effect unless it precedes -t, -th
        traceResponses = true;
        ++consumed_args;
      } else if (arg.equals("-u") && has_value) {
        username = args[consumed_args + 1];
        consumed_args += 2;
      } else if (arg.equals("-p") && has_value) {
        password = args[consumed_args + 1];
        consumed_args += 2;
      } else if ((arg.equals("-n") || arg.equals("-s")) && has_value) {
        portname = args[consumed_args + 1];
        consumed_args += 2;
      } else if (arg.equals("-d") && has_value) {
        devicename = args[consumed_args + 1];
        consumed_args += 2;
      } else if (arg.equals("-ignore-place")) {
        Message.Finished.ignorePlaceData();
        ++consumed_args;
      } else if (arg.equals("-simulate-timer")) {
        simulateTimer = true;
        ++consumed_args;
      } else if (arg.equals("-simulate-host")) {
        simulateHost = true;
        ++consumed_args;
      } else if (arg.equals("-lanes") && has_value) {
        int nlanes = Integer.parseInt(args[consumed_args + 1]);
        SimulatedDevice.setNumberOfLanes(nlanes);
        SimulatedClientSession.setNumberOfLanes(nlanes);
        consumed_args += 2;
      } else if (arg.equals("-pace") && has_value) {
        SimulatedDevice.
            setStagingTime(Integer.parseInt(args[consumed_args + 1]));
        consumed_args += 2;
      } else if (arg.equals("-reset-on-ready")) {
        TheJudgeDevice.setResetOnReady(true);
        ++consumed_args;
      } else if (arg.equals("-reset-on-race-over")) {
        TheJudgeDevice.setResetOnRaceOver(true);
        ++consumed_args;
      } else if (arg.equals("-delay-reset-after-race")
          || arg.equals("-reset-delay-on-race-over")) {
        long millis = 1000 * Integer.parseInt(args[consumed_args + 1]);
        NewBoldDevice.setPostRaceDisplayDurationMillis(millis);
        TimerDeviceCommon.setPostRaceDisplayDurationMillis(millis);
        consumed_args += 2;
      } else if (arg.equals("-skip-enhanced-format")) {
        FastTrackDevice.attemptEnhancedFormat = false;
        ++consumed_args;
      } else if (arg.equals("-skip-read-features")) {
        MicroWizard.attemptReadFeatures = false;
        ++consumed_args;
      } else if (arg.equals("-min-gate-time") && has_value) {
        TimerDeviceCommon.setMinimumGateTimeMillis(
            Integer.parseInt(args[consumed_args + 1]));
        consumed_args += 2;
      } else if (arg.equals("-record")) {
        recording = true;
        ++consumed_args;
      } else if (arg.equals("-playback") && has_value) {
        playback = true;
        PlaybackSerialPortWrapper.setFilename(args[consumed_args + 1]);
        consumed_args += 2;
      } else {
        usage();
        System.exit(1);
      }
    }
    String base_url = null;
    if (consumed_args < args.length) {
      base_url = args[consumed_args];
    }

    if (!showGui) {
      if (base_url == null && !simulateHost) {
        usage();
        System.exit(1);
      }
      if (username == null) {
        username = "Timer";
      }
      if (password == null) {
        password = "";
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

    ConnectorImpl connector = new ConnectorImpl(traceMessages);

    SimulatedClientSession simulatedSession
        = simulateHost ? new SimulatedClientSession() : null;
    try {
      TimerGui timerGui = null;
      if (showGui) {
        timerGui = startTimerGui(connector, base_url,
                                 username, password, simulatedSession,
                                 traceMessages, traceHeartbeats, traceResponses);
      } else {
        final ClientSession clientSession
            = simulatedSession == null ? new ClientSession(base_url)
              : simulatedSession;
        HttpTask.start(username, password, clientSession,
                       traceMessages, traceHeartbeats, traceResponses, connector,
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

      TimerTask timerTask = new TimerTask(portname, devicename, timerGui,
                                          connector);
      if (simulateTimer) {
        timerTask.setSimulatedTimer();
      }
      if (recording) {
        timerTask.setRecording();
      }
      if (playback) {
        timerTask.setPlayback();
      }
      timerTask.run();
    } catch (Throwable t) {
      LogWriter.stacktrace(t);
    }
  }

  private static TimerGui startTimerGui(ConnectorImpl connector, String base_url,
                                        String username, String password,
                                        ClientSession simulatedSession,
                                        boolean traceMessages,
                                        boolean traceHeartbeats,
                                        boolean traceResponses) {
    final TimerGui timerGui = new TimerGui(connector, traceMessages,
                                           traceHeartbeats, traceResponses);
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        timerGui.show();
      }
    });
    if (base_url != null) {
      timerGui.setUrl(base_url);
    }
    if (username != null || password != null) {
      timerGui.setRoleAndPassword(username, password);
    }
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
          e.printStackTrace();
        }
        httpTask.sendIdentified(
            nlanes, timerTask.device().getClass().getSimpleName(),
            timerTask.device().getTimerIdentifier());
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
            // TODO: details
            t.printStackTrace();
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
            t.printStackTrace();
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
