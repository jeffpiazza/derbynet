package org.jeffpiazza.derby;

import java.util.*;
import java.text.*;

public class Timestamp {
  private static final SimpleDateFormat yyyy_mm_dd_hh_mm_ss
      = new SimpleDateFormat("yyyy-MM-dd_HH:mm:ss");
  private static final SimpleDateFormat hh_mm_ss
      = new SimpleDateFormat("HH:mm:ss.SSS");

  public static String string() {
    // return yyyy_mm_dd_hh_mm_ss.format(Calendar.getInstance().getTime());
    return yyyy_mm_dd_hh_mm_ss.format(new Date(System.currentTimeMillis()));
  }

  public static String brief() {
    return hh_mm_ss.format(new Date(System.currentTimeMillis()));
  }
}
