package org.jeffpiazza.derby;

import java.awt.Desktop;
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

  public static LogWriter.FileAndPrintWriter makeLogFile() throws IOException {
    final String yyyymmdd_hhmm = (new SimpleDateFormat("yyyyMMdd-HHmm")).format(
        Calendar.getInstance().getTime());
    File logFile;
    PrintWriter writer;
    try {
      writer = LogFileFactory.makeLogFile(logFile = new File(logFileDirectory,
                                  "timer-" + yyyymmdd_hhmm + ".log"));
    } catch (IOException ex) {
      writer = LogFileFactory.makeLogFile(logFile = File.createTempFile("timer-", ".log"));
    }
    return new LogWriter.FileAndPrintWriter(logFile, writer);
  }

  public static PrintWriter makeLogFile(File logfile) throws IOException {
    System.err.println("Starting log file " + logfile.getAbsolutePath());
    return new PrintWriter(new BufferedWriter(new FileWriter(logfile)),
                           /*autoflush*/ true);
  }
}
