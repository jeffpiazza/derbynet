package org.jeffpiazza.derby;

import java.awt.Desktop;
import org.w3c.dom.Element;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.logging.Level;
import java.util.logging.Logger;

// TODO: Suppress heartbeats with uninteresting responses
public class LogWriter {
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

  private static File logFile;
  private static PrintWriter writer;

  public static void initialize() {
    if (writer == null) {
      try {
        makeLogFile();
        info("Started at " + Timestamp.string());
        (new Thread() {
          @Override
          public void run() {
            try {
              long seconds = (new Date(System.currentTimeMillis())).getSeconds();
              Thread.sleep(1000 * (60 - seconds));
            } catch (InterruptedException ex) {
            }
            while (true) {
              try {
                Thread.sleep(60000);
              } catch (InterruptedException ex1) {
              }
              info(Timestamp.string());
            }
          }
        }).start();
      } catch (IOException ex) {
        System.err.println("*** Unable to create log file");
      }
    }
  }

  private static void makeLogFile() throws IOException {
    final String yyyymmdd_hhmm = (new SimpleDateFormat("yyyyMMdd-HHmm")).format(
        Calendar.getInstance().getTime());
    try {
      writer = makeLogFile(logFile
          = new File(logFileDirectory, "timer-" + yyyymmdd_hhmm + ".log"));
    } catch (IOException ex) {
      writer = makeLogFile(logFile = File.createTempFile("timer-", ".log"));
    }
  }

  private static PrintWriter makeLogFile(File logfile) throws IOException {
    System.err.println("Starting log file " + logfile.getAbsolutePath());
    return new PrintWriter(new BufferedWriter(new FileWriter(logfile)),
                           /*autoflush*/ true);
  }


  protected static final String INCOMING = "<-- ";
  protected static final String OUTGOING = "--> ";
  protected static final String INTERNAL = "INT ";

  protected static final String SIMULATION_CHANNEL = "*";
  protected static final String INFO_CHANNEL = "I";
  protected static final String SERIAL_CHANNEL = "S\t";
  protected static final String HTTP_CHANNEL = "H\t\t";

  protected static final SimpleDateFormat hh_mm_ss
      = new SimpleDateFormat("HH:mm:ss.SSS");

  private static void write(String s, String channelString,
                            String directionString) {
    if (writer != null) {
      writer.println("+" + hh_mm_ss.format(new Date(System.currentTimeMillis()))
          + channelString + "\t" + directionString
          + s.replace("\r", "\\r"));
    }
  }

  public static void info(String s) {
    write(s, INFO_CHANNEL, "");
  }

  public static void httpMessage(Message m, String params) {
    write(m.asParameters(), HTTP_CHANNEL, OUTGOING);
  }

  public static void httpResponse(Element response) {
    httpResponse(XmlSerializer.serialized(response));
  }

  public static void httpResponse(String s) {
    write(s, HTTP_CHANNEL, INCOMING);
  }

  // TODO A bunch of these calls should probably be to serial()
  public static void trace(String s) {
    write(s, HTTP_CHANNEL, INTERNAL);
  }

  public static void serial(String s) {
    write(s, SERIAL_CHANNEL, INTERNAL);
  }

  public static void serialIn(String s) {
    write(s, SERIAL_CHANNEL, INCOMING);
  }

  public static void serialOut(String s) {
    write(s, SERIAL_CHANNEL, OUTGOING);
  }

  public static void simulationLog(String msg) {
    System.out.println();
    System.out.println(msg);
    if (writer != null) {
      writer.println();
    }
    write(msg, SIMULATION_CHANNEL, "");
  }

  public static void stacktrace(Throwable t) {
    if (writer != null) {
      t.printStackTrace(writer);
    }
  }

  public static void showLogFile() {
    try {
      Desktop.getDesktop().open(logFile.getParentFile());
    } catch (IOException ex) {
      Logger.getLogger(LogWriter.class.getName()).log(Level.SEVERE, null, ex);
    }
  }

  public static String laneMaskString(int laneMask, int nlanes) {
    int lane_count = nlanes;
    if (nlanes == 0) {
      lane_count = 32 - Integer.numberOfLeadingZeros(laneMask);
    }
    StringBuilder sb = new StringBuilder();
    sb.append('[');
    for (int lane = 0; lane < lane_count; ++lane) {
      if (lane > 0) {
        sb.append(' ');
      }
      if ((laneMask & (1 << lane)) != 0) {
        sb.append(1 + lane);
      } else {
        sb.append('-');
      }
    }
    sb.append(']');
    return sb.toString();
  }
}
