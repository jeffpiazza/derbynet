package org.jeffpiazza.derby.devices;

import java.text.DecimalFormat;
import jssc.SerialPortException;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

import java.util.Random;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.Message;

// For testing the web server and the derby-timer framework, simulate
// a device class
public class SimulatedDevice extends TimerDeviceBase
    implements RemoteStartInterface {
  private HeatRunner runningHeat = null;

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
    return Flag.lanes.value();
  }

  @Override
  public String getTimerIdentifier() {
    return null;
  }

  @Override
  public void prepareHeat(int roundid, int heat, int laneMask)
      throws SerialPortException {
    synchronized (this) {
      if (runningHeat == null) {
        System.out.println(
            "STAGING:  heat " + heat + " of roundid " + roundid + ": "
            + LogWriter.laneMaskString(laneMask, Flag.lanes.value()));
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
    if (runningHeat != null) {
      runningHeat.cancel();
      runningHeat = null;
    }
  }

  @Override
  public boolean hasRemoteStart() {
    return true;
  }

  @Override
  public void remoteStart() throws SerialPortException {
    System.out.println("SimulatedDevice.remoteStart called");
  }

  private long pollCount = 0;

  @Override
  public void poll() throws SerialPortException {
    ++pollCount;
    if ((pollCount % 1000) == 0
        || (pollCount < 1000 && (pollCount % 100) == 0)
        || pollCount < 10) {
      LogWriter.serial("SimulatedDevice.poll");
    }
  }

  private static final DecimalFormat decimalFormat = new DecimalFormat("#.####");

  public class HeatRunner implements Runnable {
    private int roundid;
    private int heat;
    private int lanemask;
    private boolean canceled = false;

    public HeatRunner(int roundid, int heat, int lanemask) {
      this.roundid = roundid;
      this.heat = heat;
      this.lanemask = lanemask;
    }

    public int roundid() {
      return roundid;
    }

    public int heat() {
      return heat;
    }

    public synchronized void cancel() {
      this.canceled = true;
    }

    public void run() {
      pause(Flag.pace.value());
      synchronized (this) {
        if (canceled) {
          return;
        }
      }
      LogWriter.serialOut("SimulatedDevice.run()");
      final String goMessage = "GO!       heat " + heat
          + " of roundid " + roundid + " begins.";
      System.out.println(goMessage);
      LogWriter.serialIn(goMessage);
      invokeRaceStartedCallback();
      LogWriter.serialOut("pause");
      pause(4);  // 4 seconds for a pretty slow race
      final String completeMessage = "COMPLETE: heat " + heat
          + " of roundid " + roundid + " finishes.";
      System.out.println(completeMessage);
      LogWriter.serialIn(completeMessage);
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
