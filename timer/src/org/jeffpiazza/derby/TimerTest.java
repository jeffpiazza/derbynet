package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

public class TimerTest implements TimerDevice.RaceFinishedCallback, TimerDevice.StartingGateCallback {
    public static TimerDevice identifyTimerDevice(SerialPort port) {
        System.out.println("Trying " + port.getPortName());
        try {
            SerialPortWrapper wrapper = new SerialPortWrapper(port);

            // TODO-----------
            TimerDevice[] devices = { new ChampDevice(wrapper),
                                      new FastTrackDevice(wrapper) };

            for (TimerDevice device : devices) {
                try {
                    if (device.probe()) {
                        return device;
                    }
                } catch (Throwable t) {
                    t.printStackTrace();
                }
            }
        } catch (Throwable t) {
            t.printStackTrace();
        }

        return null;
    }

    public static TimerDevice identifyTimerDevice() {
		File[] files = new File("/dev").listFiles(new FilenameFilter() {
				public boolean accept(File dir, String name) {
					return name.startsWith("tty") && !name.equals("tty") && name.contains("usb");
				}
			});

        System.out.println("identifyTimerDevice: " + files.length);  // TODO

		for (File f : files) {
            SerialPort port = new SerialPort(f.getPath());
            System.out.println("Opening " + f.getPath());
            try {
                if (!port.openPort()) {
                    System.out.println("Can't open port");
                    return null;
                }
            } catch (SerialPortException spe) {
                System.out.println("Caught exception: " + spe);
                continue;
            }

            TimerDevice device = identifyTimerDevice(port);
            if (device != null) {
                return device;
            }

			try {
				port.closePort();
			}
			catch (SerialPortException exc) {
				System.out.println("Caught exception " + exc + " while closing");
			}
        }

        return null;
    }

    private volatile boolean raceRunning = false;

    public void raceFinished(String[] results) {
        raceRunning = false;
        System.out.print("*** Race Finished: ");
        for (String r : results) {
            System.out.print(" " + r);
        }
        System.out.println();
    }

    public void startGateChange(boolean isOpen) {
        System.out.println("*** Start gate is " + (isOpen ? "open" : "closed"));
    }

    public void runTestLoop(TimerDevice device) throws SerialPortException {
        while (true) {
            device.prepareHeat(7);
            raceRunning = true;
            
            long heatDeadline = System.currentTimeMillis() + 20000;
            while (raceRunning && System.currentTimeMillis() < heatDeadline) {
                device.poll();
                try {
                    Thread.sleep(250);  // Poll 4 times per second
                } catch (Exception exc) {}
            }

            try {
                Thread.sleep(4000);  // 4-second pause between heats
            } catch (Exception exc) {}
        }
    }

    public void runTest() {
        System.out.println("Hello");

        TimerDevice device;
        while ((device = identifyTimerDevice()) == null)
            ;

        try {
            System.out.println("*** Identified " + device.getClass().getName());
            System.out.println("*** Device reports " + device.getNumberOfLanes() + " lane(s).");

            device.registerStartingGateCallback(this);
            device.registerRaceFinishedCallback(this);

            runTestLoop(device);
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    public static void main(String[] args) {
        (new TimerTest()).runTest();
    }
}
