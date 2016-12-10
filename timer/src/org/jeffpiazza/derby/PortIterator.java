package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.*;

public class PortIterator implements Iterator<SerialPort> {
  private String[] portNames;
  private int index;
  private boolean simulateDevice = false;

  public PortIterator(String[] portnames) {
    this.portNames = portnames;
    this.index = 0;
  }

  public PortIterator(String portname, boolean simulateDevice) {
    this(portname == null ? defaultPortNames() : new String[]{portname});
    if (simulateDevice) {
      this.portNames = new String[] { "Simulated Device" };
      this.simulateDevice = true;
    }
  }

  public PortIterator() {
    this(defaultPortNames());
  }

  public boolean hasNext() {
    return index < portNames.length;
  }

  public SerialPort next() {
    if (simulateDevice) {
      return null;
    }
    return new SerialPort(portNames[index++]);
  }

  public void remove() {
    throw new UnsupportedOperationException();
  }

  private static String[] mapPathNames(File[] files) {
    String[] pathnames = new String[files.length];
    for (int i = 0; i < files.length; ++i) {
      pathnames[i] = files[i].getPath();
    }
    Arrays.sort(pathnames);
    return pathnames;
  }

  private static String[] defaultPortNames() {
    if (isWindows()) {
      return SerialPortList.getPortNames();
    } else if (isLinux()) {
      return mapPathNames(new File("/dev").listFiles(new FilenameFilter() {
        public boolean accept(File dir, String name) {
          File sys = new File(new File("/sys/class/tty"), name);
          if (!sys.exists()) {
            return false;
          }
          File device = new File(sys, "device");
          if (!device.exists()) {
            return false;
          }
          File driver = new File(device, "driver");
          return driver.exists();
        }
      }));
    } else {
      return mapPathNames(new File("/dev").listFiles(new FilenameFilter() {
        public boolean accept(File dir, String name) {
          return name.startsWith("tty.");
        }
      }));
    }
  }

  private static boolean isWindows() {
    return System.getProperty("os.name").toLowerCase().contains("win");
  }

  private static boolean isLinux() {
    return System.getProperty("os.name").toLowerCase().equals("linux");
  }

  /*
  private static boolean isMac() {
    return System.getProperty("os.name").toLowerCase().equals("mac os x");
  }
   */
}
