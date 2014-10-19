package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.lang.reflect.*;
import java.util.ArrayList;

public class DeviceFinder {
  public DeviceFinder(Class<? extends TimerDevice>[] deviceClasses) {
    this.deviceClasses = deviceClasses;
  }

  public DeviceFinder() {
    this(allDeviceClasses);
  }

  public DeviceFinder(String s) {
    this(deviceClassesMatching(s));
  }

  public TimerDevice findDevice(SerialPort port) {
    try {
      if (!port.openPort()) {
        return null;
      }
    } catch (SerialPortException spe) {
      System.out.println(spe.getExceptionType());
      return null;
    } catch (Throwable t) {
      t.printStackTrace();
      return null;
    }

    TimerDevice found = null;
    try {
      found = findDeviceWithOpenPort(port);
      return found;
    } catch (Throwable t) {
      t.printStackTrace();
    } finally {
      if (found == null) {
        try { port.closePort(); }
        catch (Throwable t) {
          System.err.println("Exception closing port: ");
          t.printStackTrace();
        }
      }
    }

    return null;
  }
          

  private TimerDevice findDeviceWithOpenPort(SerialPort port) throws Exception {
    SerialPortWrapper portWrapper =
      new SerialPortWrapper(port, LogFileFactory.makeLogFile());
    for (Class<? extends TimerDevice> deviceClass : deviceClasses) {
      System.out.println("    " + deviceClass.getName());  // TODO: Cleaner logging
      Constructor<? extends TimerDevice> constructor =
          deviceClass.getConstructor(SerialPortWrapper.class);
      TimerDevice device = constructor.newInstance(portWrapper);
      if (device.probe()) {
        return device;
      }
    }

    return null;
  }

  private static Class<? extends TimerDevice>[] deviceClassesMatching(String s) {
    ArrayList<Class<? extends TimerDevice>> classes =
        new ArrayList<Class<? extends TimerDevice>>();
    for (Class<? extends TimerDevice> cl : allDeviceClasses) {
      if (cl.getName().toLowerCase().endsWith(s.toLowerCase())) {
        classes.add(cl);
      }
    }
    if (classes.size() == 0) {
      System.err.println("No device classes match " + s + ":");
      for (Class<? extends TimerDevice> cl : allDeviceClasses) {
        System.err.println("   " + cl.getName());
      }
    }
    return (Class<? extends TimerDevice>[]) classes.toArray();
  }

  public static final Class<? extends TimerDevice>[] allDeviceClasses =
  (Class<? extends TimerDevice>[]) new Class[] {
    ChampDevice.class,
    FastTrackDevice.class
  };

  private Class<? extends TimerDevice>[] deviceClasses;
};
