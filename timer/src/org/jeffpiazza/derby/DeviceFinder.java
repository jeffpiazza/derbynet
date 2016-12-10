package org.jeffpiazza.derby;

import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.devices.*;
import org.jeffpiazza.derby.gui.TimerGui;

import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.Arrays;

public class DeviceFinder {

  public DeviceFinder(String s, boolean useSimulatedTimer) {
    this.deviceClasses = deviceClassesMatching(s, useSimulatedTimer);
  }

  @SuppressWarnings("unchecked")
  private static Class<? extends TimerDevice>[]
      deviceClassesMatching(String s, boolean useSimulatedTimer) {
    if (s == null && !useSimulatedTimer) {
      return AllDeviceTypes.scannableDeviceClasses;
    }
    ArrayList<Class<? extends TimerDevice>> classes
        = new ArrayList<Class<? extends TimerDevice>>();
    if (useSimulatedTimer) {
      classes.add(SimulatedDevice.class);
    } else {
      for (Class<? extends TimerDevice> cl : AllDeviceTypes.allDeviceClasses) {
        if (cl.getName().toLowerCase().endsWith(s.toLowerCase())) {
          classes.add(cl);
        }
      }
    }
    if (classes.size() == 0) {
      System.err.println(
          "**** No device classes match " + s
          + "; use -h option to get a list of recognized classes");
    }
    return (Class<? extends TimerDevice>[]) classes.toArray(
        new Class[classes.size()]);
  }

  // This member variable defines the TimerDevice classes over which this
  // PortIterator will iterate.
  private Class<? extends TimerDevice>[] deviceClasses;

  public Class<? extends TimerDevice>[] deviceClasses() {
    return Arrays.copyOf(deviceClasses, deviceClasses.length);
  }

  public TimerDevice findDevice(SerialPort port, TimerGui timerGui,
                                boolean recording, LogWriter w) {
    try {
      if (port != null && !port.openPort()) {
        return null;
      }
    } catch (SerialPortException spe) {
      System.out.println(spe.getExceptionType());
      if (timerGui != null) {
        timerGui.markSerialPortWontOpen();
      }
      return null;
    } catch (Throwable t) {
      t.printStackTrace();
      return null;
    }

    TimerDevice found = null;
    try {
      found = findDeviceWithOpenPort(port, timerGui, recording, w);
      return found;
    } catch (Throwable t) {
      t.printStackTrace();
    } finally {
      if (found == null) {
        try {
          // Performs a removeEventListener, too.
          if (port != null) {
            port.closePort();
          }
        } catch (Throwable t) {
          System.err.println("Exception closing port: ");
          t.printStackTrace();
        }
      }
    }

    return null;
  }

  @SuppressWarnings("unchecked")
  private TimerDevice findDeviceWithOpenPort(SerialPort port, TimerGui timerGui,
                                             boolean recording, LogWriter w)
      throws Exception {
    SerialPortWrapper portWrapper
        = recording ? new RecordingSerialPortWrapper(port, w)
          : new SerialPortWrapper(port, w);
    for (Class<? extends TimerDevice> deviceClass : deviceClasses) {
      if (timerGui != null) {
        timerGui.setTimerClass(
            (Class<? extends TimerDevice>) deviceClass);
      }
      System.out.println("    " + deviceClass.getName());
      w.serialPortLogInternal("Trying " + deviceClass.getName() + " on "
          + (port == null ? "Simulated Port" : port.getPortName()));
      Constructor<? extends TimerDevice> constructor
          = deviceClass.getConstructor(SerialPortWrapper.class);
      TimerDevice device = constructor.newInstance(portWrapper);
      if (device.probe()) {
        System.out.println("*** Identified!!");
        w.serialPortLogInternal("*** Successfully identified ***");
        return device;
      }
    }

    // We're no longer interested in this wrapper, so the wrapper's no longer
    // interested in events from the port.
    if (port != null) {
      port.removeEventListener();
    }
    return null;
  }
}
