package org.jeffpiazza.derby.devices;

import java.awt.Color;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.util.Arrays;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.AllSerialPorts;
import org.jeffpiazza.derby.Connector;
import org.jeffpiazza.derby.HttpTask;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.gui.TimerGui;
import org.jeffpiazza.derby.serialport.PlaybackSerialPortWrapper;
import org.jeffpiazza.derby.serialport.RecordingSerialPortWrapper;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;

public class TimerTask implements Runnable, HttpTask.TimerHealthCallback {
  private TimerGui timerGui;
  private Connector connector;
  private static final int PORT_NORMAL = 0;
  private static final int PORT_RECORDING = 1;
  private static final int PORT_PLAYBACK = 2;
  private int port_record_playback = PORT_NORMAL;
  private TimerDevice device;

  // Flag that's set when a click from the GUI changes the serial port or timer
  // class, and then cleared at the top of the identifyTimerDevice() loop.
  private boolean userIntervened = false;

  public TimerTask(String portname, String devicename, TimerGui timerGui,
                   Connector connector) {
    this.timerGui = timerGui;
    this.connector = connector;

    timerClasses = new ChoosableList<Class<? extends TimerDevice>>(
        AllDeviceTypes.allTimerDeviceClasses());
    if (devicename != null) {
      timerClasses.choose(AllDeviceTypes.getDeviceClass(devicename));
    }
    if (timerGui != null) {
      timerGui.updateTimerClasses(this, timerClasses.allCandidates());
    }

    // serialPorts will get (re)populated each time through identifyTimerDevice
    serialPorts = new ChoosableList<String>(new String[0]);
    if (portname != null) {
      serialPorts.choose(portname);
    }
  }

  // This is called when simulating the device class, i.e., for exercising
  // the http task and web server
  public void setSimulatedTimer() {
    timerClasses.choose(SimulatedDevice.class);
    serialPorts.choose("");
  }

  public void setRecording() {
    port_record_playback = PORT_RECORDING;
  }

  public void setPlayback() {
    port_record_playback = PORT_PLAYBACK;
    serialPorts.choose("");
  }

  public void userChoosesTimerClass(Class<? extends TimerDevice> timerClass) {
    timerClasses.choose(timerClass);
    setUserIntervened(true);
  }

  public void userChoosesSerialPort(String portName) {
    serialPorts.choose(portName);
    setUserIntervened(true);
  }

  private synchronized boolean userIntervened() {
    return userIntervened;
  }

  private synchronized void setUserIntervened(boolean v) {
    userIntervened = v;
    this.notifyAll();
  }

  public void run() {
    // This while loop ensures we revert to scanning for timers upon loss of
    // connection.
    while (true) {
      try {
        connector.setTimerTask(this);
        if (device == null) {
          device = identifyTimerDevice();
        }
        // Let the connector know we've (possibly) identified the device
        connector.setTimerTask(this);
        runDevicePollingLoop();
      } catch (TimerDevice.LostConnectionException lce) {
        System.err.println("Lost connection!");
        if (device != null) {
          String msg = "No response from timer in "
              + device.getPortWrapper().millisSinceLastContact() + "ms.";
          LogWriter.serial(msg);
          device.invokeMalfunctionCallback(true, msg);
        } else {
          LogWriter.trace("LostConnectionException with no device!");
        }
        if (timerGui != null) {
          // Note that this status message will get replaced almost immediately
          // as the new scan starts
          timerGui.setSerialStatus("Lost connection", Color.red,
                                   TimerGui.icon_trouble);
        }
      } catch (Throwable ex) {
        Throwable cause = ex.getCause();
        if (cause == null) {
          cause = ex;
        }
        LogWriter.trace(
            "** Timer loop restarted due to "
            + cause.getClass().getName() + ": " + ex.getMessage());
        LogWriter.stacktrace(ex);
      } finally {
        if (device != null) {
          device.close();
          device = null;
        }
      }
    }
  }

  // A collection of choices, possible with an explicit choice already made.
  // If a choice has been made, then it's the only candidate, otherwise all
  // possibilities from the list are available.
  private static class ChoosableList<T> {
    private T[] list;
    // null to indicate nothing chosen, otherwise a value that may or may not
    // be an element of list.
    private T chosen;

    public ChoosableList(T[] list) {
      this.list = list;
    }

    public synchronized void choose(T choice) {
      this.chosen = choice;
    }

    public synchronized T chosen() {
      return chosen;
    }

    public synchronized void repopulate(T[] list) {
      this.list = list;
    }

    public synchronized T[] candidates() {
      if (chosen != null) {
        T[] copy = Arrays.copyOf(list, 1);
        copy[0] = chosen;
        return copy;
      } else {
        return list;
      }
    }

    public T[] allCandidates() {
      return list;
    }
  }

  private ChoosableList<String> serialPorts;
  private ChoosableList<Class<? extends TimerDevice>> timerClasses;

  // Starts repeatedly scanning all serial ports (or just the one with a given
  // name) for all devices (or just the one with a given name), and returns a
  // TimerDevice if/when it identifies one.  Shows its progress through the
  // TimerGui, if available.
  public TimerDevice identifyTimerDevice()
      throws SerialPortException, IOException {
    while (true) {
      setUserIntervened(false);
      if (timerGui != null) {
        timerGui.setSerialStatus("Initializing list of serial ports",
                                 Color.black, TimerGui.icon_unknown);
      }
      // The set of serial ports on the machine may change over time, e.g.
      // as USB converters are plugged in or unplugged, so it's important to
      // re-scan the machine each time through the loop.
      serialPorts.repopulate(AllSerialPorts.getNames());
      if (timerGui != null) {
        timerGui.updateSerialPorts(this, serialPorts.allCandidates());
        timerGui.setSerialStatus("Scanning for connected timer");
        timerGui.deselectAll();
      }

      for (String portName : serialPorts.candidates()) {
        if (userIntervened()) {
          break;
        }
         TimerDevice device = tryOnePortName(portName);
         if (device != null) {
           return device;
         }
      }

      long deadline = System.currentTimeMillis() + 10000;
      if (timerGui != null) {
        timerGui.deselectAll();
        timerGui.setSerialStatus("(Pausing between scans)");
      }
      synchronized (this) {
        while (!userIntervened() && System.currentTimeMillis() < deadline) {
          try {
            this.wait(deadline - System.currentTimeMillis());
          } catch (InterruptedException ex) {
          }
        }
      }
    }
  }

  private TimerDevice tryOnePortName(String portName) throws SerialPortException {
    SerialPort port = portName.isEmpty() ? null : new SerialPort(portName);
    if (timerGui != null) {
      timerGui.setSerialPort(portName);
    }
    try {
      if (port != null) {
        System.out.println(port.getPortName());
        if (!open(port)) {
          if (timerGui != null) {
            timerGui.markSerialPortWontOpen();
          }
          port = null;  // So we don't try to close it in the finally clause
          return null;
        }
      }
      TimerPortWrapper portWrapper;
      switch (port_record_playback) {
        case PORT_NORMAL:
        default:
          portWrapper = new SerialPortWrapper(port);
          break;
        case PORT_RECORDING:
          portWrapper = new RecordingSerialPortWrapper(port);
          break;
        case PORT_PLAYBACK:
          portWrapper = new PlaybackSerialPortWrapper();
          break;
      }

      TimerDevice device = tryOnePortWrapper(portWrapper);
      if (device != null) {
        // 'finally' clause will close port, if it's set; since we
        // actually found a timer, we want to leave the port open.
        port = null;
        return device;
      } else {
        portWrapper.abandon();
      }
    } finally {
      close(port);
    }
    return null;
  }

  public TimerDevice tryOnePortWrapper(TimerPortWrapper portWrapper)
      throws SerialPortException {
    for (Class<? extends TimerDevice> timerClass
         : timerClasses.candidates()) {
      if (userIntervened()) {
        break;
      }
      if (timerGui != null) {
        timerGui.setTimerClass(timerClass);
      }
      TimerDevice device = maybeProbeOneDevice(
          makeTimerDeviceInstance(timerClass, portWrapper));
      if (device != null) {
        return device;
      }
    }
    return null;
  }

  private TimerDevice maybeProbeOneDevice(TimerDevice device)
      throws SerialPortException {
    if (device == null) {
      return null;
    }
    Class<? extends TimerDevice> timerClass = device.getClass();
    TimerPortWrapper portWrapper = device.getPortWrapper();
    System.out.print("    " + timerClass.getSimpleName());
    if (!device.canBeIdentified()) {
      if (timerClass != timerClasses.chosen()) {
        // Unless the user chose this class, treat as a failed probe without
        // bothering to probe.
        LogWriter.serial(
            "Skipping " + timerClass.getSimpleName()
            + " on " + portWrapper.getPortName());
        System.out.println(" (skipped)");
        return null;
      } else {
        // probe() method likely sets up serial parameters and/or recognizers,
        // even if it doesn't actually probe anything.
        device.probe();
        String msg = "Assuming " + timerClass.getSimpleName()
            + ", because positive identification is not possible.";
        System.out.println();
        System.out.println(msg);
        LogWriter.serial(msg);
        return device;
      }
    } else {
      LogWriter.serial(
          "Trying " + timerClass.getSimpleName()
          + " on " + portWrapper.getPortName());

      if (device.probe()) {
        String msg = "*** Identified as a(n) " + timerClass.getSimpleName();
        System.out.println();
        System.out.println(msg);
        LogWriter.serial(msg);
        return device;
      } else {
        System.out.println();
        return null;  // next device class
      }
    }
  }

  private void close(SerialPort port) {
    try {
      if (port != null) {
        // Performs a removeEventListener, too.
        port.closePort();
      }
    } catch (Throwable t) {
      LogWriter.trace("Exception closing port");
      LogWriter.stacktrace(t);
    }
  }

  private boolean open(SerialPort port) {
    try {
      return port.openPort();
    } catch (SerialPortException spe) {
      LogWriter.serial("openPort fails for " + port.getPortName() + ": "
          + spe.getExceptionType());
      System.err.println(spe.getExceptionType());
      return false;
    }
  }

  private TimerDevice makeTimerDeviceInstance(
      Class<? extends TimerDevice> deviceClass, TimerPortWrapper portWrapper) {
    try {
      Constructor<? extends TimerDevice> constructor
          = deviceClass.getConstructor(TimerPortWrapper.class);
      return constructor.newInstance(portWrapper);
    } catch (Throwable ex) {
      LogWriter.stacktrace(ex);
    }
    return null;
  }

  // Continuously polls the timer device for messages, and checks for timeouts
  // of expected race results.
  private void runDevicePollingLoop()
      throws SerialPortException, TimerDevice.LostConnectionException {
    while (!userIntervened()) {
      device.poll();
      confirmDevice();
      try {
        Thread.sleep(50); // ms.
      } catch (Exception exc) {
      }
    }
  }

  private void confirmDevice() {
    if (device != null && timerGui != null) {
      timerGui.confirmDevice(device.hasEverSpoken());
    }
  }

  public synchronized void injectDevice(TimerDevice device) {
    this.device = device;
  }

  public synchronized TimerDevice device() {
    return device;
  }

  @Override
  public int getTimerHealth() {
    if (device() == null) {
      return UNHEALTHY;
    }
    if (device().hasEverSpoken()) {
      return HEALTHY;
    }
    return PRESUMED_HEALTHY;
  }
}
