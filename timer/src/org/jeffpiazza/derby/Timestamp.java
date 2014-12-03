package org.jeffpiazza.derby;

import java.util.*;
import java.text.*;

public class Timestamp {
  public static String string() {
    return new SimpleDateFormat("yyyy-MM-dd_HH:mm:ss")
        .format(Calendar.getInstance().getTime());
  }
}
