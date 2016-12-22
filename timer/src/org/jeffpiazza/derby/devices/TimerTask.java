package org.jeffpiazza.derby.devices;

import java.awt.Color;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.HttpTask;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.AllSerialPorts;
import org.jeffpiazza.derby.RecordingSerialPortWrapper;
import org.jeffpiazza.derby.SerialPortWrapper;
import org.jeffpiazza.derby.TimerMain;
import org.jeffpiazza.derby.gui.TimerGui;

public class TimerTask implements Runnable, HttpTask.TimerHealthCallback {
  private TimerGui timerGui;
  private LogWriter logwriter;
  private TimerMain.ConnectorImpl connector;
  private boolean recording = false;
  private TimerDevice device;

  // Flag that's set when a click from the GUI changes the serial port or timer
  // class, and then cleared at the top of the identifyTimerDevice() loop.
  private boolean userIntervened = false;

  public TimerTask(String portname, String devicename, TimerGui timerGui,
                   LogWriter logwriter, TimerMain.ConnectorImpl connector) {
    this.timerGui = timerGui;
    this.logwriter = logwriter;
    this.connector = connector;

    timerClasses = new ChoosableList<Class<? extends TimerDevice>>(
        AllDeviceTypes.allDeviceClasses);
    if (devicename != null) {
      timerClasses.choose(timerClassByName(devicename));
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

  // This is called when simulating the device class
  public void setSimulatedTimer() {
    timerClasses.choose(SimulatedDevice.class);
    serialPorts.choose("");
  }

  public void setRecording() {
    this.recording = true;
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

  // TODO resumeScan method from GUI un-chooses timer class and serial port, and away you go.
  // Assumes control of the thread to fully manage the serial device.
  public void run() {
    // This while loop ensures we revert to scanning for timers upon loss of
    // connection.
    while (true) {
      try {
        device = identifyTimerDevice();
        connector.setTimerTask(this);
        runDevicePollingLoop();
      } catch (TimerDevice.LostConnectionException lce) {
        System.out.println("Lost connection!");
        String msg = "No response from timer in "
            + device.getPortWrapper().millisSinceLastContact() + "ms.";
        device.getPortWrapper().logWriter().serialPortLogInternal(msg);
        device.invokeMalfunctionCallback(true, msg);
        if (timerGui != null) {
          // Note that this status message will get replaced almost immediately
          // as the new scan starts
          timerGui.setSerialStatus("Lost connection", Color.red,
                                   TimerGui.icon_trouble);
        }
      } catch (Throwable ex) {
        Logger.getLogger(TimerTask.class.getName()).log(Level.SEVERE, null,
                                                        ex);
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

  private Class<? extends TimerDevice> timerClassByName(String s) {
    for (Class<? extends TimerDevice> timerClass : timerClasses.allCandidates()) {
      if (timerClass.getName().toLowerCase().endsWith(s.toLowerCase())) {
        return timerClass;
      }
    }
    System.err.println(
        "**** No device classes match " + s
        + "; use -h option to get a list of recognized classes");
    return null;
  }

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
        SerialPort port = portName.isEmpty() ? null : new SerialPort(portName);
        SerialPortWrapper portWrapper = null;
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
              continue;
            }
            portWrapper = recording
                          ? new RecordingSerialPortWrapper(port, logwriter)
                          : new SerialPortWrapper(port, logwriter);
          }
          for (Class<? extends TimerDevice> timerClass
               : timerClasses.candidates()) {
            if (userIntervened()) {
              break;
            }
            System.out.print("    " + timerClass.getSimpleName());
            logwriter.serialPortLogInternal(
                "Trying " + timerClass.getSimpleName() + " on "
                + (port == null ? "Simulated Port" : port.getPortName()));
            if (timerGui != null) {
              timerGui.setTimerClass(timerClass);
            }
            TimerDevice device
                = makeTimerDeviceInstance(timerClass, portWrapper);
            device = maybeProbeOneDevice(device);
            if (device != null) {
              if (timerGui != null) {
                timerGui.confirmDevice();
              }
              // 'finally' clause will close port, if it's set; since we
              // actually found a timer, we want to leave the port open.
              port = null;
              return device;
            }
          }
          // If falling out of the loop here, then we're no longer interested
          // in this wrapper, so the wrapper's no longer interested in events
          // from the port
          if (port != null) {
            port.removeEventListener();
          }
        } finally {
          close(port);
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

  private TimerDevice maybeProbeOneDevice(TimerDevice device)
      throws SerialPortException {
    if (device == null) {
      return null;
    }
    Class<? extends TimerDevice> timerClass = device.getClass();
    if (!device.canBeIdentified()) {
      if (timerClass == timerClasses.chosen()) {
        // If the user chose this class, then treat as a succeeded probe.
        // Otherwise, treat as a failed probe.
        String msg = "Assuming " + timerClass.getSimpleName()
            + ", because positive identification is not possible.";
        System.out.println();
        System.out.println(msg);
        logwriter.serialPortLogInternal(msg);
        return device;
      } else {
        System.out.println(" (skipped)");
        return null;
      }
    } else if (device.probe()) {
      String msg = "*** Identified as a(n) " + timerClass.getSimpleName();
      System.out.println();
      System.out.println(msg);
      logwriter.serialPortLogInternal(msg);
      return device;
    } else {
      System.out.println();
      return null;  // next device class
    }
  }

  private void close(SerialPort port) {
    try {
      if (port != null) {
        // Performs a removeEventListener, too.
        port.closePort();
      }
    } catch (Throwable t) {
      System.err.println("Exception closing port: ");
      t.printStackTrace();
    }
  }

  private boolean open(SerialPort port) {
    try {
      return port.openPort();
    } catch (SerialPortException spe) {
      System.out.println(spe.getExceptionType());
      return false;
    }
  }

  private TimerDevice makeTimerDeviceInstance(
      Class<? extends TimerDevice> deviceClass, SerialPortWrapper portWrapper) {
    try {
      Constructor<? extends TimerDevice> constructor
          = deviceClass.getConstructor(SerialPortWrapper.class);
      return constructor.newInstance(portWrapper);
    } catch (NoSuchMethodException ex) {
    } catch (SecurityException ex) {
    } catch (InstantiationException ex) {
    } catch (IllegalAccessException ex) {
    } catch (IllegalArgumentException ex) {
    } catch (InvocationTargetException ex) {
    }
    return null;
  }

  // Continuously polls the timer device for messages, and checks for timeouts
  // of expected race results.
  private void runDevicePollingLoop()
      throws SerialPortException, TimerDevice.LostConnectionException {
    while (!userIntervened()) {
      device.poll();
      try {
        Thread.sleep(50); // ms.
      } catch (Exception exc) {
      }
    }
  }

  public synchronized TimerDevice device() {
    return device;
  }

  @Override
  public boolean isTimerHealthy() {
    return device() != null;
  }
}
