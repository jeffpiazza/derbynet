package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;

// Two threads: timer polling loop runs on main thread, HttpTask runs on another
// thread

public class TimerMain {
  public static long raceTimeoutMillis = 11000;
  private static volatile long raceDeadline = -1;

  public static void usage() {
    System.err.println("Usage: [options] <base-url>");
    System.err.println("   -u <user>: Specify username for authenticating to web server");
    System.err.println("   -p <password>: Specify password for authenticating to web server");
    System.err.println("   -t: Trace non-heartbeat messages sent");
    System.err.println("   -th: Trace heartbeat messages sent");
    System.err.println("   -r: Show responses to traced messages");
    System.err.println("   -n <port name>: Use specified port name instead of searching");
    System.err.println("   -d <device name>: Use specified device instead of trying to identify");
  }

  public static void main(String[] args) {
    String username = "RaceCoordinator";
    String password = "doyourbest";
    String portname = null;
    String devicename = null;
    HttpTask.MessageTracer traceMessages = null;
    HttpTask.MessageTracer traceHeartbeats = null;
    boolean traceResponses = false;

    LogWriter logwriter = null;
    try {
      logwriter = new LogWriter();
    } catch (Throwable t) {
      t.printStackTrace();
      return;
    }

    int consumed_args = 0;
    while (consumed_args + 1 < args.length) {
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
      } else {
        usage();
        System.exit(1);
      }
    }

    if (consumed_args + 1 != args.length) {
      usage();
      System.exit(1);
    }

    String base_url = args[consumed_args];

    try {
      sayHelloAndPoll(base_url, username, password, identifyTimerDevice(portname, devicename, logwriter));
    } catch (Throwable t) {
      t.printStackTrace();
    }
  }

  public static TimerDevice identifyTimerDevice(String portname, String devicename, LogWriter logwriter)
      throws SerialPortException, IOException {
    final DeviceFinder deviceFinder =
        devicename == null ? new DeviceFinder() : new DeviceFinder(devicename);

    while (true) {
      PortIterator ports = portname == null ? new PortIterator() : new PortIterator(portname);
      while (ports.hasNext()) {
        SerialPort port = ports.next();
        System.out.println(port.getPortName());  // TODO: Better logging
        TimerDevice device = deviceFinder.findDevice(port, logwriter);
        if (device != null) {
          return device;
        }
      }
      try {
        Thread.sleep(10000);  // Wait 10 seconds before trying again
      } catch (Throwable t) {}
    }
  }

  public static void sayHelloAndPoll(String base_url, String username, String password,
                                     final TimerDevice device) throws Exception {
    final HttpTask httpTask = new HttpTask(base_url, username, password);

    wireTogether(httpTask, device);

    int nlanes = device.getNumberOfLanes();

    boolean sentHello = false;
    while (!sentHello) {
      try {
        httpTask.send(new Message.Hello(nlanes));
        sentHello = true;
      } catch (Throwable t) {
        t.printStackTrace();
      }
    }

    System.out.println("Starting HTTP thread");
    (new Thread(httpTask)).start();

    runDevicePollingLoop(device);
  }

  public static void wireTogether(final HttpTask httpTask, final TimerDevice device) {
    httpTask.registerHeatReadyCallback(new HttpTask.HeatReadyCallback() {
        public void heatReady(int laneMask) {
          try {
            device.prepareHeat(laneMask);
          } catch (Throwable t) {
            // TODO: details
            try {
              httpTask.send(new Message.Malfunction("Can't ready timer."));
            } catch (Throwable tt) {
            }
          }
        }
      });

    httpTask.registerAbortHeatCallback(new HttpTask.AbortHeatCallback() {
        public void abortHeat() {
          System.out.println(Timestamp.string() + ": AbortHeat received");
          raceDeadline = -1;
          try {
            device.abortHeat();
          } catch (Throwable t) {
            t.printStackTrace();
          }
        }
      });

    device.registerRaceFinishedCallback(new TimerDevice.RaceFinishedCallback() {
        public void raceFinished(Message.LaneResult[] results) {
          // Rely on recipient to ignore if not expecting any results
          try {
            raceDeadline = -1;
            System.out.println(Timestamp.string() + ": Race finished");
            httpTask.send(new Message.Finished(results));
          } catch (Throwable t) {
          }
        }
      });

    device.registerRaceStartedCallback(new TimerDevice.RaceStartedCallback() {
        public void raceStarted() {
          try {
            raceDeadline = System.currentTimeMillis() + raceTimeoutMillis;
            System.out.println(Timestamp.string() + ": Race started");
            httpTask.send(new Message.Started());
          } catch (Throwable t) {
          }
        }
      });
  }

  private static void runDevicePollingLoop(TimerDevice device) throws SerialPortException {
    while (true) {
      device.poll();
      if (!(raceDeadline < 0 || System.currentTimeMillis() < raceDeadline)) {
        // TODO: Race timed out, not sure what to do.
        // Some choices:
        // - Send empty results back to web server.
        // - Repeat prepareHeat.
        // - Nothing, as now.
        System.err.println(Timestamp.string() + ": ****** Race timed out *******");
        raceDeadline = -1;
      }
      try {
        Thread.sleep(50);  // ms.
      } catch (Exception exc) {}
    }
  }
}
