package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;

// TODO: UNFINISHED!!!!!!!!!!!!!!!!!!!!!

// scan for timer.  If not found, repeat.

// send HELLO; keep trying until successful

// do forever:
//   poll (HEARTBEAT) until a STARTED
//   keep polling
//   start timer
//   poll timer until results in.
//   send FINISHED

// Two threads: one for timer, one for heartbeat

// Meanwhile, timer thread takes a device plug-in, and polls timer continuously.
// On HEAT-READY, arm the timer
//  On gate open, send STARTED message, and (Champ) if timer armed, start internal timer to abort after 10 seconds.
// (Maybe just send heartbeat with gate state if not racing?)
// On gate close, send heartbeat
// On timer result, if racing send FINISHED
// On 10-second elapsed, force race finish, send FINISHED 

public class TimerMain {
    private HttpTask task;

    private TimerMain(HttpTask task) {
        this.task = task;
    }

    public static void usage() {
        System.err.println("Usage: [-u <user>] [-p <password>] [-l <nlanes>] [-t] [-th] [-r] <base-url>");
        System.err.println("   -u, -p: Specify username and password for authenticating to web server");
        System.err.println("   -l: Specify number of lanes to report to web server");
        System.err.println("   -t: Trace non-heartbeat messages sent");
        System.err.println("   -th: Trace heartbeat messages sent");
        System.err.println("   -r: Show responses to traced messages");
    }

    private static Class[] knownTimerDeviceClasses = { FastTrackDevice.class, ChampDevice.class };

    public static void main(String[] args) {
        String username = "RaceCoordinator";
        String password = "doyourbest";
        int nlanes = 3;
        StdoutMessageTrace traceMessages = null;
        StdoutMessageTrace traceHeartbeats = null;
        boolean traceResponses = false;

        int consumed_args = 0;
        while (consumed_args + 1 < args.length) {
            if (args[consumed_args].equals("-u") && consumed_args + 2 < args.length) {
                username = args[consumed_args + 1];
                consumed_args += 2;
            } else if (args[consumed_args].equals("-p") && consumed_args + 2 < args.length) {
                password = args[consumed_args + 1];
                consumed_args += 2;
            } else if (args[consumed_args].equals("-l") && consumed_args + 2 < args.length) {
                try {
                    nlanes = Integer.parseInt(args[consumed_args + 1]);
                } catch (Throwable t) {
                    t.printStackTrace();
                }
            } else if (args[consumed_args].equals("-t")) {
                traceMessages = new StdoutMessageTrace();
                ++consumed_args;
            } else if (args[consumed_args].equals("-th")) {
                traceHeartbeats = new StdoutMessageTrace();
                ++consumed_args;
            } else if (args[consumed_args].equals("-r")) {
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

        if (traceMessages != null) {
            traceMessages.traceResponses = traceResponses;
        }
        if (traceHeartbeats != null) {
            traceHeartbeats.traceResponses = traceResponses;
        }

        String base_url = args[consumed_args];

        try {
            //            (new TimerMain(new HttpTask(base_url, username, password,
            //                                        traceMessages, traceHeartbeats)))
            //            .runTest(nlanes);
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    private static TimerDevice identifyDevice(SerialPortWrapper wrapper) throws SerialPortException {
        while (true) {
            for (Class deviceClass : knownTimerDeviceClasses) {
                try {
                    System.out.println("Trying " + deviceClass.getName());
                    TimerDevice device = 
                        (TimerDevice) deviceClass.getConstructor(SerialPortWrapper.class).newInstance(wrapper);
                    if (device.probe()) {
                        return device;
                    }
                } catch (Throwable t) {
                    t.printStackTrace();
                }
            }
            try {
                Thread.sleep(10000);  // Wait 10 seconds before trying again
            } catch (Throwable t) {}
        }
    }

    public static void experiment(String base_url, String username, String password,
                                  SerialPort port) throws Exception {
        SerialPortWrapper wrapper = new SerialPortWrapper(port);

        final TimerDevice device = identifyDevice(wrapper);
        int nlanes = device.getNumberOfLanes();

        final TimerDeviceTask deviceTask = new TimerDeviceTask(device);
        final HttpTask httpTask = new HttpTask(base_url, username, password);

        (new Thread(deviceTask)).start();
        // (new Thread(httpTask)).start();

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

        device.registerRaceFinishedCallback(new TimerDevice.RaceFinishedCallback() {
                public void raceFinished(Message.LaneResult[] results) {
                    // Rely on recipient to ignore if not expecting any results
                    try {
                        httpTask.send(new Message.Finished(results));
                    } catch (Throwable t) {
                    }
                }
            });
        device.registerStartingGateCallback(new TimerDevice.StartingGateCallback() {
                public void startGateChange(boolean isOpen) {
                    try {
                        if (isOpen /* && timer armed? */) {
                            httpTask.send(new Message.Started());
                        } else {
                            httpTask.send(new Message.Heartbeat());
                        }
                    } catch (Throwable t) {
                    }
                }
            });

        // TODO: handler for ABORT message: device.abortHeat();

        boolean sentHello = false;
        while (!sentHello) {
            try {
                httpTask.send(new Message.Hello(nlanes));
                sentHello = true;
            } catch (Throwable t) {
                t.printStackTrace();
            }
        }

        httpTask.run();
    }
}
