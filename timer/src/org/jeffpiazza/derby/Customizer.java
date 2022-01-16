// The jar file can be customized (by including a custom.url and/or custom.args
// file) so that it defaults to a particular URL or applies some default
// command line arguments.  This is the class that reads those customizations.
package org.jeffpiazza.derby;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Scanner;

public class Customizer {
  // Returns null if no custom.url file is present in the jar file
  public static String getCustomUrl() {
    InputStream stream = Customizer.class.getClassLoader()
        .getResourceAsStream("custom.url");
    if (stream == null) {
      return null;
    }
    Scanner sc = new Scanner(stream);
    return sc.next();
  }

  // Returns null if no custom.args file is present in the jar file
  public static String[] getCustomArgs() {
    InputStream stream = Customizer.class.getClassLoader()
        .getResourceAsStream("custom.args");
    if (stream == null) {
      return null;
    }
    BufferedReader reader = new BufferedReader(new InputStreamReader(stream));

    ArrayList<String> args = new ArrayList<String>();
    try {
    String line;
    int sp;
    while ((line = reader.readLine()) != null) {
      if (!line.isEmpty() && line.charAt(0) == '-' &&
          (sp = line.indexOf(' ')) > 0) {
        args.add(line.substring(0, sp - 1));
        args.add((line.substring(sp).trim()));
      } else {
        args.add(line.trim());
      }
    }
    } catch (IOException ex) {
      LogWriter.stacktrace(ex);
      System.exit(1);
    }

    return args.toArray(new String[0]);
  }
}
