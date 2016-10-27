package org.jeffpiazza.derby;

import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.devices.*;
import org.jeffpiazza.derby.gui.TimerGui;

import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;

public class DeviceFinder {
  public DeviceFinder(Class<? extends TimerDevice>[] deviceClasses) {
    this.deviceClasses = deviceClasses;
  }

  public DeviceFinder() {
    this(allDeviceClasses);
  }

  public DeviceFinder(boolean withFake) {
    this(allDeviceClassesWithFake(withFake));
  }

  public DeviceFinder(String s) {
    this(deviceClassesMatching(s));
  }

  public TimerDevice findDevice(SerialPort port, TimerGui timerGui,
                                LogWriter w) {
    try {
      if (!port.openPort()) {
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
      found = findDeviceWithOpenPort(port, timerGui, w);
      return found;
    } catch (Throwable t) {
      t.printStackTrace();
    } finally {
      if (found == null) {
        try {
          port.closePort();
        } catch (Throwable t) {
          System.err.println("Exception closing port: ");
          t.printStackTrace();
        }
      }
    }

    return null;
  }

  public Class<? extends TimerDevice>[] deviceClasses() {
    return Arrays.copyOf(deviceClasses, deviceClasses.length);
  }

  public static void listDeviceClassNames() {
    for (Class<? extends TimerDevice> cl : allDeviceClasses) {
      System.err.println("         " + cl.getName().substring(1 + cl.
          getName().lastIndexOf(".")));
    }
  }

  @SuppressWarnings("unchecked")
  private TimerDevice findDeviceWithOpenPort(SerialPort port,
                                             TimerGui timerGui, LogWriter w)
      throws Exception {
    SerialPortWrapper portWrapper = new SerialPortWrapper(port, w);
    for (Class<? extends TimerDevice> deviceClass : deviceClasses) {
      if (timerGui != null) {
        timerGui.setTimerClass(
            (Class<? extends TimerDevice>) deviceClass);
      }
      System.out.println("    " + deviceClass.getName());
      w.serialPortLogInternal("Trying " + deviceClass.getName() + " on "
          + port.getPortName());
      Constructor<? extends TimerDevice> constructor
          = deviceClass.getConstructor(SerialPortWrapper.class);
      TimerDevice device = constructor.newInstance(portWrapper);
      if (device.probe()) {
        System.out.println("*** Identified!!");
        w.serialPortLogInternal("*** Successfully identified ***");
        return device;
      }
    }

    return null;
  }

  @SuppressWarnings("unchecked")
  private static Class<? extends TimerDevice>[] deviceClassesMatching(String s) {
    ArrayList<Class<? extends TimerDevice>> classes
        = new ArrayList<Class<? extends TimerDevice>>();
    for (Class<? extends TimerDevice> cl : allDeviceClasses) {
      if (cl.getName().toLowerCase().endsWith(s.toLowerCase())) {
        classes.add(cl);
      }
    }
    if (classes.size() == 0) {
      System.err.println(
          "**** No device classes match " + s +
              "; use -h option to get a list of recognized classes");
    }
    return (Class<? extends TimerDevice>[]) classes.toArray(
        new Class[classes.size()]);
  }

  @SuppressWarnings("unchecked")
  private static final Class<? extends TimerDevice>[] allDeviceClasses
      = (Class<? extends TimerDevice>[]) new Class[]{
        ChampDevice.class,
        FastTrackDevice.class
      };

  @SuppressWarnings("unchecked")
  public static Class<? extends TimerDevice>[] allDeviceClassesWithFake(
      boolean withFake) {
    ArrayList<Class<? extends TimerDevice>> classes
        = new ArrayList<Class<? extends TimerDevice>>();
    Collections.addAll(classes, allDeviceClasses);
    if (withFake) {
      classes.add(FakeDevice.class);
    }
    return (Class<? extends TimerDevice>[]) classes.toArray(
        new Class[classes.size()]);
  }

  private Class<? extends TimerDevice>[] deviceClasses;
}
