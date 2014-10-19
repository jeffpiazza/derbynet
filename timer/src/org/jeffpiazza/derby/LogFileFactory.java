package org.jeffpiazza.derby;

import java.io.*;

public class LogFileFactory {
  public static PrintWriter makeLogFile() throws IOException {
    return makeLogFile("timer.log");
  }

  public static PrintWriter makeLogFile(String path) throws IOException {
    File logfile = new File(path);
    if (!logfile.exists()) {
      logfile.createNewFile();
    }
    return new PrintWriter(new BufferedWriter(new FileWriter(logfile)),
                           /*autoflush*/true);
  }
}

