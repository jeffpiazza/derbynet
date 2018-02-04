package org.jeffpiazza.derby.devices;

import java.lang.reflect.Method;

public class AllDeviceTypes {
  // The universe of all possible device classes
  @SuppressWarnings(value = "unchecked")
  public static final Class<? extends TimerDevice>[] allDeviceClasses
      = (Class<? extends TimerDevice>[]) new Class[]{
        SmartLineDevice.class,
        FastTrackDevice.class,
        OlderFastTrackDevice.class,
        TheJudgeDevice.class,
        NewBoldDevice.class,
        DerbyTimerDevice.class,
        MiscJunkDevice.class,
        DerbyMagicDevice.class
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
                         + (human == null ? "" : (": " + human)));
    }
  }
}
