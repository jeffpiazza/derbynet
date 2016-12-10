package org.jeffpiazza.derby.devices;

import java.lang.reflect.Method;

public class AllDeviceTypes {
  // The device classes that support probing to identify the attached device
  @SuppressWarnings(value = "unchecked")
  public static final Class<? extends TimerDevice>[] scannableDeviceClasses
      = (Class<? extends TimerDevice>[]) new Class[]{
        ChampDevice.class,
        FastTrackDevice.class};

  // The universe of all possible device classes
  @SuppressWarnings(value = "unchecked")
  public static final Class<? extends TimerDevice>[] allDeviceClasses
      = (Class<? extends TimerDevice>[]) new Class[]{
        ChampDevice.class,
        FastTrackDevice.class,
        OlderFastTrackDevice.class,
        TheJudgeDevice.class
      };

  public static String toHumanString(Class<? extends TimerDevice> type) {
    try {
      Method m = type.getMethod("toHumanString");
      return (String) m.invoke(null);
    } catch (Exception ex) {
      return null;
    }
  }

  public static void listDeviceClassNames() {
    for (Class<? extends TimerDevice> cl : allDeviceClasses) {
      String human = toHumanString(cl);
      System.err.println("         " + cl.getSimpleName()
                         + (cl == null ? "" : (": " + human)));
    }
  }
}
