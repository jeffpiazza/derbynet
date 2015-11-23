package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.*;

public class PortIterator implements Iterator<SerialPort> {

  public PortIterator(String[] portnames) {
    this.portNames = portnames;
    this.index = 0;
  }

  public PortIterator(String portname) {
    this(new String[] { portname });
  }

  public PortIterator(File[] files) {
    this(mapPathNames(files));
  }

  public PortIterator() {
    this(defaultPortNames());
  }

  public boolean hasNext() { return index < portNames.length; }

  public SerialPort next() { return new SerialPort(portNames[index++]); }

  public void remove() { throw new UnsupportedOperationException(); }

  private static String[] mapPathNames(File[] files) {
    String[] pathnames = new String[files.length];
    for (int i = 0; i < files.length; ++i) {
      pathnames[i] = files[i].getPath();
    }
    return pathnames;
  }

  private static String[] defaultPortNames() {
    if (isWindows()) {
      return SerialPortList.getPortNames();
    } else {
      return mapPathNames(new File("/dev").listFiles(new FilenameFilter() {
          public boolean accept(File dir, String name) {
            return name.startsWith("tty.") && !name.equals("tty")
            // Mac devices: /dev/tty.KeySerial1, /dev/tty.USA19Hfd12P1.1
            /* && (name.contains("usb") || name.contains("USB")) */;
          }
        }));
    }
  }

  private static boolean isWindows() {
    return System.getProperty("os.name").toLowerCase().contains("win");
  }

  String[] portNames;
  int index;
}
