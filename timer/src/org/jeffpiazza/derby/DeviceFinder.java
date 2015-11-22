package org.jeffpiazza.derby;

import jssc.*;
import org.jeffpiazza.derby.gui.TimerGui;

import java.lang.reflect.*;
import java.util.ArrayList;
import java.util.Arrays;

public class DeviceFinder {
  public DeviceFinder(Class<? extends TimerDevice>[] deviceClasses) {
    this.deviceClasses = deviceClasses;
  }

  public DeviceFinder() {
    this(allDeviceClasses);
  }
  public DeviceFinder(boolean withFake) { this(allDeviceClassesWithFake(withFake)); }
  public DeviceFinder(String s) {
    this(deviceClassesMatching(s));
  }

  public TimerDevice findDevice(SerialPort port, TimerGui timerGui, LogWriter w) {
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
      System.err.println("         " + cl.getName().substring(1 + cl.getName().lastIndexOf(".")));
    }
  }

  private TimerDevice findDeviceWithOpenPort(SerialPort port, TimerGui timerGui, LogWriter w) throws Exception {
    SerialPortWrapper portWrapper = new SerialPortWrapper(port, w);
    for (Class<? extends TimerDevice> deviceClass : deviceClasses) {
      if (timerGui != null) {
        timerGui.setTimerClass(deviceClass);
      }
      System.out.println("    " + deviceClass.getName());  // TODO: Cleaner logging
      Constructor<? extends TimerDevice> constructor =
          deviceClass.getConstructor(SerialPortWrapper.class);
      TimerDevice device = constructor.newInstance(portWrapper);
      if (device.probe()) {
        System.out.println("*** Identified!!");
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
      System.err.println("**** No device classes match " + s + "; use -h option to get a list of recognized classes");
    }
    return (Class<? extends TimerDevice>[]) classes.toArray(new Class[classes.size()]);
  }

  private static final Class<? extends TimerDevice>[] allDeviceClasses =
      (Class<? extends TimerDevice>[]) new Class[]{
          ChampDevice.class,
          FastTrackDevice.class
      };

  public static Class<? extends TimerDevice>[] allDeviceClassesWithFake(boolean withFake) {
    ArrayList<Class<? extends TimerDevice>> classes =
        new ArrayList<Class<? extends TimerDevice>>();
    for (Class<? extends TimerDevice> cl : allDeviceClasses) {
      classes.add(cl);
    }
    if (withFake) {
      classes.add(FakeDevice.class);
    }
    return (Class<? extends TimerDevice>[]) classes.toArray(new Class[classes.size()]);
  }

  private Class<? extends TimerDevice>[] deviceClasses;
};
