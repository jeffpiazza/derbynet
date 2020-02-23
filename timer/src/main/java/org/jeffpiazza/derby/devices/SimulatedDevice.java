package org.jeffpiazza.derby.devices;

import java.text.DecimalFormat;
import jssc.SerialPortException;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

import java.util.Random;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;

// For testing the web server and the derby-timer framework, simulate
// a device class
public class SimulatedDevice extends TimerDeviceBase implements TimerDevice {
  private static int nlanes = 0;
  private static int staging_time = 10;  // seconds;
  private HeatRunner runningHeat = null;

  public static void setNumberOfLanes(int n) {
    nlanes = n;
  }

  public static void setStagingTime(int nsec) {
    staging_time = nsec;
  }

  private Random random;

  public SimulatedDevice(SerialPortWrapper portWrapper) {
    super(portWrapper);
    this.random = new Random();
  }

  public static String toHumanString() {
    return "Simulated Timer Device";
  }

  @Override
  public boolean probe() throws SerialPortException {
    // 50% chance of "discovering" our fake device on a given port
    return random.nextFloat() < 0.50;
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return nlanes;
  }

  @Override
  public String getTimerIdentifier() { return null; }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    synchronized (this) {
      if (runningHeat == null) {
        System.out.println("STAGING:  heat " + heat + " of roundid " + roundid + ": "
            + LogWriter.laneMaskString(laneMask, nlanes));
        runningHeat = new HeatRunner(roundid, heat, laneMask);
        (new Thread(runningHeat)).start();
      } // TODO Confirm roundid/heat match runningHeat
    }
  }

  private synchronized void endRunningHeat() {
    runningHeat = null;
  }

  @Override
  public void abortHeat() throws SerialPortException {
    System.out.println("SimulatedDevice.abortHeat called");
    // TODO cancel heatrunner
  }

  private long pollCount = 0;

  @Override
  public void poll() throws SerialPortException {
    ++pollCount;
    if ((pollCount % 1000) == 0
        || (pollCount < 1000 && (pollCount % 100) == 0)
        || pollCount < 10) {
      // TODO System.out.println("SimulatedDevice.poll() called "
      //                         + pollCount + " time(s).");
    }
  }

  private static final DecimalFormat decimalFormat = new DecimalFormat("#.####");

  public class HeatRunner implements Runnable {
    private int roundid;
    private int heat;
    private int lanemask;

    public HeatRunner(int roundid, int heat, int lanemask) {
      this.roundid = roundid;
      this.heat = heat;
      this.lanemask = lanemask;
    }

    public int roundid() { return roundid; }
    public int heat() { return heat; }

    public void run() {
      pause(staging_time);
      System.out.println("GO!       heat " + heat + " of roundid " + roundid + " begins.");
      invokeRaceStartedCallback();
      pause(4);  // 4 seconds for a pretty slow race
      System.out.println("COMPLETE: heat " + heat + " of roundid " + roundid + " finishes.");
      invokeRaceFinishedCallback(roundid, heat, makeHeatResults(lanemask));
      endRunningHeat();
    }

    private Message.LaneResult[] makeHeatResults(int lanemask) {
      int nlanes = 32 - Integer.numberOfLeadingZeros(lanemask);
      Message.LaneResult[] results = new Message.LaneResult[nlanes];
      for (int lane = 0; lanemask != 0; ++lane) {
        if ((lanemask & (1 << lane)) != 0) {
          results[lane] = new Message.LaneResult();
          results[lane].time = decimalFormat.format(2.0 + (Math.random() * 2.0));
          lanemask ^= (1 << lane);
        }
      }
      return results;
    }

    public void pause(int nsec) {
      try {
        Thread.sleep(nsec * 1000);
      } catch (Throwable t) {
      }
    }
  }
}
