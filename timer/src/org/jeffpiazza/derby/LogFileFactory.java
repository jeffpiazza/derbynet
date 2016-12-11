package org.jeffpiazza.derby;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Calendar;

public class LogFileFactory {
  public static PrintWriter makeLogFile() throws IOException {
    final String yyyymmdd_hhmm = (new SimpleDateFormat("yyyyMMdd-HHmm")).format(
        Calendar.getInstance().getTime());
    try {
      return makeLogFile("timer-" + yyyymmdd_hhmm + ".log");
    } catch (IOException ex) {
      return makeLogFile(File.createTempFile("timer-", ".log"));
    }
  }

  public static PrintWriter makeLogFile(String path) throws IOException {
    File logfile = new File(path);
    if (!logfile.exists()) {
      logfile.createNewFile();
    }
    return makeLogFile(logfile);
  }

  public static PrintWriter makeLogFile(File logfile) throws IOException {
    System.err.println("Starting log file " + logfile.getAbsolutePath());
    return new PrintWriter(new BufferedWriter(new FileWriter(logfile)),
        /*autoflush*/ true);
  }
}
