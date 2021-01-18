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

  private static void setLogFileDirectory(String directory) {
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
  private static ClientSession clientSession;
  private static StringBuilder remoteLogBuffer = new StringBuilder();
  private static boolean isRemoteLogging = false;
  private static boolean hasEverHadRemoteLogging = false;
  private static long remoteLogDeadline = 0;
  private static final long kRemoteLogDeltaMs = 100;

  public static void initialize() {
    if (writer == null) {
      if (Flag.logdir.value() != null) {
        setLogFileDirectory(Flag.logdir.value());
      }
      try {
        makeLogFile();
        info("Started at " + Timestamp.string());
        // Timestamp thread
        (new Thread() {
          @Override
          public void run() {
            // Periodically wake up and add a timestamp to the log.
            // Align to a round minute boundary before starting.
            try {
              long seconds = Calendar.getInstance().get(Calendar.SECOND);
              Thread.sleep(1000 * (60 - seconds));
            } catch (InterruptedException ex) {
            }
            while (true) {
              try {
                Thread.sleep(60000);
              } catch (InterruptedException ex1) {
              }
              final String ts = Timestamp.string();
              info(ts);
              writeRemoteLogFragment("===== ", ts);
            }
          }
        }).start();

        // Remote logging thread
        (new Thread() {
          @Override
          public void run() {
            while (true) {
              String fragment;
              synchronized (remoteLogBuffer) {
                while (remoteLogBuffer.length() == 0) {
                  try {
                    if (!isRemoteLogging) {
                      // Wait indefinitely if we're not sending remote logging info.
                      // setRemoteLogging will notify if necessary.
                      remoteLogBuffer.wait();
                    } else {
                      long sleep_ms = kRemoteLogDeltaMs;
                      if (remoteLogDeadline != 0) {
                        long sleep2 = remoteLogDeadline
                            - System.currentTimeMillis();
                        if (sleep2 != 0 && sleep2 < sleep_ms) {
                          sleep_ms = sleep2;
                        }
                      }
                      remoteLogBuffer.wait(sleep_ms);
                    }
                  } catch (InterruptedException ex) {
                  }
                }
                fragment = remoteLogBuffer.toString();
                remoteLogBuffer.setLength(0);
                remoteLogDeadline = 0;
              }
              if (clientSession != null) {
                try {
                  clientSession.sendTimerLogFragment(fragment);
                } catch (IOException ex) {
                }
              }
            }
          }
        }).start();
      } catch (IOException ex) {
        System.err.println("*** Unable to create log file");
      }
    }
  }

  public static void setClientSession(ClientSession session) {
    clientSession = session;
  }

  public static void setRemoteLogging(boolean on) {
    synchronized (remoteLogBuffer) {
      isRemoteLogging = on;
      if (isRemoteLogging) {
        if (!hasEverHadRemoteLogging) {
          remoteLogBuffer.append(Timestamp.string());
          remoteLogBuffer.append("\nClient log file: ");
          remoteLogBuffer.append(logFile.toString());
          remoteLogBuffer.append("\n\n");
          hasEverHadRemoteLogging = true;
        }
        remoteLogBuffer.notifyAll();
      }
    }
  }

  public static void pauseLogging() {
    if (writer != null) {
      writer.close();
      writer = null;
    }
  }
  public static void resumeLogging() {
    if (writer == null) {
      try {
        makeLogFile();
      } catch (IOException ex) {
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

  private static void writeRemoteLogFragment(String prefix, String s) {
    synchronized (remoteLogBuffer) {
      if (isRemoteLogging) {
        if (remoteLogDeadline == 0) {
          remoteLogDeadline = System.currentTimeMillis() + kRemoteLogDeltaMs;
        }
        //remoteLogBuffer.append(hh_mm_ss.format(
        //    new Date(System.currentTimeMillis())));
        //remoteLogBuffer.append(" ");
        remoteLogBuffer.append(prefix);
        remoteLogBuffer.append(s);
        remoteLogBuffer.append("\n");
      }
    }
  }

  public static void info(String s) {
    write(s, INFO_CHANNEL, "");
  }

  public static void httpMessage(String s) {
    write(s, HTTP_CHANNEL, OUTGOING);
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
    writeRemoteLogFragment(INTERNAL, s);
  }

  public static void serialIn(String s) {
    write(s, SERIAL_CHANNEL, INCOMING);
    writeRemoteLogFragment(INCOMING, s);
  }

  public static void serialOut(String s) {
    write(s, SERIAL_CHANNEL, OUTGOING);
    writeRemoteLogFragment(OUTGOING, s);
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
