package org.jeffpiazza.derby.devices;

import java.lang.reflect.Method;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.profiles.BertDrake;
import org.jeffpiazza.derby.profiles.FastTrack;
import org.jeffpiazza.derby.profiles.Champ;
import org.jeffpiazza.derby.profiles.ChampSRM;
import org.jeffpiazza.derby.profiles.DerbyMagic9600;
import org.jeffpiazza.derby.profiles.DerbyMagic;
import org.jeffpiazza.derby.profiles.DerbyTimer;
import org.jeffpiazza.derby.profiles.FastTrackPSeries;
import org.jeffpiazza.derby.profiles.JitRacemaster;
import org.jeffpiazza.derby.profiles.NewBold;
import org.jeffpiazza.derby.profiles.Pdt;
import org.jeffpiazza.derby.profiles.TheJudge;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

public class AllDeviceTypes {
  // The universe of all possible device classes
  @SuppressWarnings(value = "unchecked")
  private static final Class<? extends TimerDevice>[] allDeviceClasses
      = (Class<? extends TimerDevice>[]) new Class[]{
        Champ.class,
        ChampSRM.class,
        FastTrack.class,
        FastTrackPSeries.class,
        TheJudge.class,
        NewBold.class,
        DerbyTimer.class,
        Pdt.class,
        DerbyMagic.class,
        DerbyMagic9600.class,
        BertDrake.class,
        JitRacemaster.class
      };
  @SuppressWarnings(value = "unchecked")
  private static final Class<? extends TimerDevice>[] legacyDeviceClasses
      = (Class<? extends TimerDevice>[]) new Class[]{
        SmartLineLegacy.class,
        FastTrackLegacy.class,
        OlderFastTrackLegacy.class,
        TheJudgeLegacy.class,
        NewBoldLegacy.class,
        DerbyTimerLegacy.class,
        MiscJunkLegacy.class,
        DerbyMagicLegacy.class,
        DerbyMagic9600Legacy.class,
        DerbyMagic19200Legacy.class,
        BertDrakeLegacy.class,
        RacemasterLegacy.class
      };
  public static final Class<? extends TimerDevice>[] allTimerDeviceClasses() {
    if (Flag.legacy_implementations.value()) {
      return legacyDeviceClasses;
    }
    return allDeviceClasses;
  }

  public static String toHumanString(Class<? extends TimerDevice> type) {
    try {
      Method m = type.getMethod("toHumanString");
      return (String) m.invoke(null);
    } catch (Exception ex) {
    }

    if (TimerDeviceWithProfile.class.isAssignableFrom(type)) {
      try {
        Method m = type.getMethod("profile");
        Profile p = (Profile) m.invoke(null);
        return p.name;
      } catch (Exception ex) {
      }
    }

    // Last resort
    return type.getName();
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
