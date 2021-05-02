package org.jeffpiazza.derby.devices;

import java.lang.reflect.Method;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.profiles.BertDrakeByProfile;
import org.jeffpiazza.derby.profiles.FastTrackByProfile;
import org.jeffpiazza.derby.profiles.ChampByProfile;
import org.jeffpiazza.derby.profiles.DerbyMagic9600ByProfile;
import org.jeffpiazza.derby.profiles.DerbyMagicByProfile;
import org.jeffpiazza.derby.profiles.DerbyTimerByProfile;
import org.jeffpiazza.derby.profiles.FastTrackPSeriesByProfile;
import org.jeffpiazza.derby.profiles.JitRacemasterByProfile;
import org.jeffpiazza.derby.profiles.NewBoldByProfile;
import org.jeffpiazza.derby.profiles.PdtByProfile;
import org.jeffpiazza.derby.profiles.TheJudgeByProfile;

public class AllDeviceTypes {
  // The universe of all possible device classes
  @SuppressWarnings(value = "unchecked")
  private static final Class<? extends TimerDevice>[] allDeviceClasses
      = (Class<? extends TimerDevice>[]) new Class[]{
        ChampByProfile.class,
        FastTrackByProfile.class,
        FastTrackPSeriesByProfile.class,
        TheJudgeByProfile.class,
        NewBoldByProfile.class,
        DerbyTimerByProfile.class,
        PdtByProfile.class,
        DerbyMagicByProfile.class,
        DerbyMagic9600ByProfile.class,
        BertDrakeByProfile.class,
        JitRacemasterByProfile.class
      };
  @SuppressWarnings(value = "unchecked")
  private static final Class<? extends TimerDevice>[] legacyDeviceClasses
      = (Class<? extends TimerDevice>[]) new Class[]{
        SmartLineDevice.class,
        FastTrackDevice.class,
        OlderFastTrackDevice.class,
        TheJudgeDevice.class,
        NewBoldDevice.class,
        DerbyTimerDevice.class,
        MiscJunkDevice.class,
        DerbyMagicDevice.class,
        DerbyMagic9600.class,
        DerbyMagic19200.class,
        BertDrakeDevice.class,
        RacemasterDevice.class
      };
  public static final Class<? extends TimerDevice>[] allTimerDeviceClasses() {
    if (Flag.beta_test.value()) {
      return allDeviceClasses;
    }
    return legacyDeviceClasses;
  }

  public static String toHumanString(Class<? extends TimerDevice> type) {
    try {
      Method m = type.getMethod("toHumanString");
      return (String) m.invoke(null);
    } catch (Exception ex) {
      return null;
    }
  }

  public static void listDeviceClassNames() {
    for (Class<? extends TimerDevice> cl : allTimerDeviceClasses()) {
      String human = toHumanString(cl);
      System.err.println("         " + cl.getSimpleName()
          + (human == null ? "" : (": " + human)));
    }
  }

  public static Class<? extends TimerDevice> getDeviceClass(String className) {
    if (className.isEmpty()) {
      return null;
    }
    for (Class<? extends TimerDevice> cl : allTimerDeviceClasses()) {
      if (cl.getName().toLowerCase().endsWith(className.toLowerCase())) {
        return cl;
      }
    }
    System.err.println(
        "**** No device classes match " + className
        + "; use -h option to get a list of recognized classes");
    return null;
  }
}
