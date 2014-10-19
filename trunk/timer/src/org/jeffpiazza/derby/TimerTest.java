// Discovers connected timer, and then simulates responding to
// commands from the web server, asking to start or abandon heats.

package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

public class TimerTest implements TimerDevice.RaceStartedCallback,
                                  TimerDevice.RaceFinishedCallback {

  public static TimerDevice identifyTimerDevice() throws SerialPortException, IOException {
    final DeviceFinder deviceFinder = new DeviceFinder();

    for (PortIterator ports = new PortIterator(); ports.hasNext(); ) {
      SerialPort port = ports.next();
      System.out.println(port.getPortName());  // TODO: Better logging
      TimerDevice device = deviceFinder.findDevice(port);
      if (device != null) {
        return device;
      }
    }

    return null;
  }

  private volatile boolean raceRunning = false;
  private volatile long raceDeadline;

  public synchronized void raceStarted() {
    System.out.println("*** Race started");
    raceDeadline = System.currentTimeMillis() + 15000;
  }

  public void raceFinished(Message.LaneResult[] results) {
    System.out.print("*** Race Finished: ");
    raceRunning = false;
    for (Message.LaneResult r : results) {
      if (r != null) {
        System.out.print(" " + r.time + "(" + r.place + ")");
      } else {
        System.out.println("--- null received ---");
      }
    }
    System.out.println();
  }

  public void runTestLoop(TimerDevice device) throws SerialPortException {
	int mask = 0;
    while (true) {
      -- mask;
      if (mask <= 0) { mask = 7; } // TODO: Generalize

      device.prepareHeat(mask);
      raceRunning = true;
      // Semaphore value meaning there's no deadline
      raceDeadline = -1;

      while (raceRunning && (raceDeadline < 0 || System.currentTimeMillis() < raceDeadline)) {
        device.poll();
        try {
          Thread.sleep(50);  // ms.
        } catch (Exception exc) {}
      }

      if (raceRunning) {
        raceRunning = false;
        System.out.println("Abandoning heat (wrong choice!)");
      }

      try {
		System.out.println("Pausing before starting a new heat.");
        Thread.sleep(4000);  // 4-second pause between heats
      } catch (Exception exc) {}
    }
  }

  public void runTest() {
    try {
      System.out.println("Starting TimerTest");

      TimerDevice device;
      while ((device = identifyTimerDevice()) == null)
        ;

      System.out.println("*** Identified " + device.getClass().getName());
      System.out.println("*** Device reports " + device.getNumberOfLanes() + " lane(s).");

      device.registerRaceStartedCallback(this);
      device.registerRaceFinishedCallback(this);

      System.out.println();
      System.out.println();
      System.out.println();
      System.out.println("Running test loop");
      runTestLoop(device);
      System.out.println("Quitting!");
    } catch (Throwable t) {
      t.printStackTrace();
    }
  }

  public static void main(String[] args) {
    (new TimerTest()).runTest();
  }
}
