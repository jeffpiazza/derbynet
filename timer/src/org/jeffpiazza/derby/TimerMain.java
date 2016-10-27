package org.jeffpiazza.derby;

import jssc.*;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.gui.TimerGui;

import javax.swing.*;
import org.jeffpiazza.derby.devices.TimerTask;

// Three threads for three "actors":
// timer polling loop runs on main thread,
// HttpTask runs on another thread,
// GUI event dispatch runs on a third thread.
public class TimerMain {
  public static void usage() {
    System.err.println("Usage: [options] <base-url>");
    System.err.println("   -h or -help or --help: This message");
    System.err.println("   -x: Run headless, without gui.");
    System.err.println(
        "   -u <user>: Specify username for authenticating to web server");
    System.err.println(
        "   -p <password>: Specify password for authenticating to web server");
    System.err.println("   -t: Trace non-heartbeat messages sent");
    System.err.println("   -th: Trace heartbeat messages sent");
    System.err.println("   -r: Show responses to traced messages");
    System.err.println(
        "   -n <port name>: Use specified port name instead of searching");
    System.err.println(
        "   -d <device name>: Use specified device instead of trying to identify");
    System.err.println("      Known devices:");
    DeviceFinder.listDeviceClassNames();
  }

  public static void main(String[] args) {
    String username = "RaceCoordinator";
    String password = "doyourbest";
    String portname = null;
    String devicename = null;
    HttpTask.MessageTracer traceHeartbeats = null;
    boolean traceResponses = false;
    boolean showGui = true;
    boolean fakeDevice = false;

    LogWriter logwriter = null;
    try {
      logwriter = new LogWriter();
    } catch (Throwable t) {
      t.printStackTrace();
      return;
    }

    // Include HTTP traffic in the timer log:
    HttpTask.MessageTracer traceMessages = logwriter;

    int consumed_args = 0;
    while (consumed_args < args.length && args[consumed_args].startsWith("-")) {
      if (args[consumed_args].equals("-u") && consumed_args + 2 < args.length) {
        username = args[consumed_args + 1];
        consumed_args += 2;
      } else if (args[consumed_args].equals("-p") && consumed_args + 2 < args.length) {
        password = args[consumed_args + 1];
        consumed_args += 2;
      } else if (args[consumed_args].equals("-n") && consumed_args + 2 < args.length) {
        portname = args[consumed_args + 1];
        consumed_args += 2;
      } else if (args[consumed_args].equals("-d") && consumed_args + 2 < args.length) {
        devicename = args[consumed_args + 1];
        consumed_args += 2;
      } else if (args[consumed_args].equals("-t")) {
        StdoutMessageTrace smt = new StdoutMessageTrace();
        smt.traceResponses = traceResponses;
        traceMessages = new CombinedMessageTracer(smt, logwriter);
        ++consumed_args;
      } else if (args[consumed_args].equals("-th")) {
        StdoutMessageTrace smt = new StdoutMessageTrace();
        smt.traceResponses = traceResponses;
        traceHeartbeats = new CombinedMessageTracer(traceHeartbeats, logwriter);
        ++consumed_args;
      } else if (args[consumed_args].equals("-r")) { // Won't have effect unless it precedes -t, -th
        traceResponses = true;
        ++consumed_args;
      } else if (args[consumed_args].equals("-x")) {
        showGui = false;
        ++consumed_args;
      } else if (args[consumed_args].equals("-fake")) {
        fakeDevice = true;
        ++consumed_args;
      } else {
        usage();
        System.exit(1);
      }
    }
    String base_url = "localhost";
    if (consumed_args + 1 == args.length) {
      base_url = args[consumed_args];
    } else if (!showGui) {
      usage();
      System.exit(1);
    }

    ConnectorImpl connector = new ConnectorImpl(traceMessages);

    try {
      if (showGui) {
        final TimerGui timerGui = new TimerGui(traceMessages, traceHeartbeats,
                                               connector);
        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            timerGui.show();
          }
        });

        (new TimerTask(portname, devicename, fakeDevice, timerGui,
                       logwriter, connector, traceMessages)).run();
      } else {
        HttpTask.start(username, password, new ClientSession(base_url),
                       traceMessages, traceHeartbeats, connector,
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
        (new TimerTask(portname, devicename, fakeDevice, null,
                       logwriter, connector, traceMessages)).run();
      }
    } catch (Throwable t) {
      t.printStackTrace();
    }
  }

  // Allow the timer device and web server connection to come up in either
  // order, or perhaps not at all; when they're both established, wire together
  // callbacks and send hello with lane count to web server.
  public static class ConnectorImpl implements Connector {
    private HttpTask httpTask;
    private TimerTask timerTask;
    private HttpTask.MessageTracer traceMessages;

    public ConnectorImpl(HttpTask.MessageTracer traceMessages) {
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
        httpTask.sendIdentified(nlanes);
      }
    }

    // Registers callbacks that allow the httpTask and timer device to
    // communicate asynchronously.
    public static void wireTogether(final HttpTask httpTask,
                                    final TimerTask timerTask,
                                    final HttpTask.MessageTracer traceMessages) {
      if (traceMessages != null) {
        traceMessages.traceInternal(Timestamp.string() + ": Timer detected.");
      }

      httpTask.registerHeatReadyCallback(new HttpTask.HeatReadyCallback() {
        public void heatReady(int laneMask) {
          try {
            if (traceMessages != null) {
              traceMessages.traceInternal(Timestamp.string() + ": Heat ready");
            }
            timerTask.device().prepareHeat(laneMask);
          } catch (Throwable t) {
            // TODO: details
            try {
              httpTask.queueMessage(new Message.Malfunction(false,
                                                            "Can't ready timer."));
            } catch (Throwable tt) {
            }
          }
        }
      });

      httpTask.registerAbortHeatCallback(new HttpTask.AbortHeatCallback() {
        public void abortHeat() {
          if (traceMessages != null) {
            traceMessages.traceInternal(
                Timestamp.string() + ": AbortHeat received");
          }
          timerTask.clearRaceWatchdog();
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
            timerTask.startRaceWatchdog();
            if (traceMessages != null) {
              traceMessages.traceInternal(Timestamp.string() + ": Race started");
            }
            httpTask.queueMessage(new Message.Started());
          } catch (Throwable t) {
          }
        }
      });

      timerTask.device().registerRaceFinishedCallback(
          new TimerDevice.RaceFinishedCallback() {
        public void raceFinished(Message.LaneResult[] results) {
          // Rely on recipient to ignore if not expecting any results
          try {
            timerTask.clearRaceWatchdog();
            if (traceMessages != null) {
              traceMessages.
                  traceInternal(Timestamp.string() + ": Race finished");
            }
            httpTask.queueMessage(new Message.Finished(results));
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
