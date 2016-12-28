package org.jeffpiazza.derby;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Calendar;

public class LogFileFactory {
  private static File logFileDirectory
      = new File(System.getProperty("user.dir"));

  public static void setLogFileDirectory(String directory) {
    File dir = new File(directory);
    if (!dir.exists()) {
      dir.mkdirs();
    }
    if (dir.isDirectory()) {
      logFileDirectory = dir;
    } else {
      System.err.println(
          "File path " + dir.getAbsolutePath() + " is not a directory.");
    }
  }

  public static PrintWriter makeLogFile() throws IOException {
    final String yyyymmdd_hhmm = (new SimpleDateFormat("yyyyMMdd-HHmm")).format(
        Calendar.getInstance().getTime());
    try {
      return makeLogFile(new File(logFileDirectory,
                                  "timer-" + yyyymmdd_hhmm + ".log"));
    } catch (IOException ex) {
      return makeLogFile(File.createTempFile("timer-", ".log"));
    }
  }

  public static PrintWriter makeLogFile(File logfile) throws IOException {
    System.err.println("Starting log file " + logfile.getAbsolutePath());
    return new PrintWriter(new BufferedWriter(new FileWriter(logfile)),
                           /*autoflush*/ true);
  }
}
