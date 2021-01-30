// The jar file can be customized (by including a custom.url file) so that it
// defaults to a particular URL; this is the class that reads that URL.
package org.jeffpiazza.derby;

import java.io.InputStream;
import java.util.Scanner;

public class CustomUrlFinder {
  // Returns null if no custom.url file is present in the jar file
  public static String getCustomUrl() {
    InputStream stream = CustomUrlFinder.class.getClassLoader()
        .getResourceAsStream("custom.url");
    if (stream == null) {
      return null;
    }
    Scanner sc = new Scanner(stream);
    return sc.next();
  }
}
