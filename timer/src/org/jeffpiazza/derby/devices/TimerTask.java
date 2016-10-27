package org.jeffpiazza.derby.devices;

import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.DeviceFinder;
import org.jeffpiazza.derby.HttpTask;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.PortIterator;
import org.jeffpiazza.derby.TimerMain;
import org.jeffpiazza.derby.Timestamp;
import org.jeffpiazza.derby.gui.TimerGui;

public class TimerTask implements Runnable {

  private String portname;
  private String devicename;
  private boolean fakeDevice;
  private TimerGui timerGui;
  private LogWriter logwriter;
  private TimerMain.ConnectorImpl connector;
  private HttpTask.MessageTracer traceMessages;
  private TimerDevice device;

  public TimerTask(String portname, String devicename, boolean fakeDevice,
                   TimerGui timerGui, LogWriter logwriter,
                   TimerMain.ConnectorImpl connector,
                   HttpTask.MessageTracer traceMessages) {
    this.portname = portname;
    this.devicename = devicename;
    this.fakeDevice = fakeDevice;
    this.timerGui = timerGui;
    this.logwriter = logwriter;
    this.connector = connector;
    this.traceMessages = traceMessages;
  }

  // Assumes control of the thread to fully manage the serial device.
  //
  // TODO Upon a loss of connection, fall back to identifying the timer device.
  // Note that re-connecting an unplugged cable, probably the most common
  // loss-of-connection situation, may result in a new serial port showing up,
  // so we have to scan.
  public void run() {
    try {
      device = identifyTimerDevice();
      connector.setTimerTask(this);
      runDevicePollingLoop();
    } catch (Throwable ex) {
      Logger.getLogger(TimerTask.class.getName()).log(Level.SEVERE, null,
                                                      ex);
    }
  }

  // Starts repeatedly scanning all serial ports (or just the one with a given
  // name) for all devices (or just the one with a given name), and returns a
  // TimerDevice if/when it identifies one.  Shows its progress through the
  // TimerGui, if available.
  public TimerDevice identifyTimerDevice()
      throws SerialPortException, IOException {
    final DeviceFinder deviceFinder
        = devicename != null ? new DeviceFinder(devicename)
          : fakeDevice ? new DeviceFinder(true)
            : new DeviceFinder();
    if (timerGui != null) {
      timerGui.initializeTimerClasses(deviceFinder);
    }
    while (true) {
      PortIterator ports = portname == null ? new PortIterator()
                           : new PortIterator(portname);
      if (timerGui != null) {
        timerGui.updateSerialPorts();
        timerGui.setSerialStatus("Scanning for connected timer");
      }
      while (ports.hasNext()) {
        SerialPort port = ports.next();
        if (timerGui != null) {
          timerGui.setSerialPort(port);
        }
        System.out.println(port.getPortName());
        TimerDevice device = deviceFinder.findDevice(port, timerGui, logwriter);
        if (device != null) {
          if (timerGui != null) {
            timerGui.confirmDevice(port, device.getClass());
          }
          return device;
        }
      }
      if (timerGui != null) {
        timerGui.deselectAll();
        timerGui.setSerialStatus("(Pausing between scans)");
      }
      try {
        Thread.sleep(10000); // Wait 10 seconds before trying again
      } catch (Throwable t) {
      }
    }
  }

  // Continuously polls the timer device for messages, and checks for timeouts
  // of expected race results.
  private void runDevicePollingLoop() throws SerialPortException {
    while (true) {
      device.poll();
      if (!(raceDeadline < 0 || System.currentTimeMillis() < raceDeadline)) {
        // TODO: Race timed out, not sure what to do.
        // Some choices:
        // - Send empty results back to web server.
        // - Repeat prepareHeat.
        // - Nothing, as now.
        String msg = Timestamp.string() + ": ****** Race timed out *******";
        traceMessages.traceInternal(msg);
        System.err.println(msg);
        clearRaceWatchdog();
      }
      try {
        Thread.sleep(50); // ms.
      } catch (Exception exc) {
      }
    }
  }

  public TimerDevice device() {
    return device;
  }

  private volatile long raceDeadline = -1;
  private final long raceTimeoutMillis = 11000;

  public void startRaceWatchdog() {
    raceDeadline = System.currentTimeMillis() + raceTimeoutMillis;
  }

  public void clearRaceWatchdog() {
    raceDeadline = -1;
  }
}
