package org.jeffpiazza.derby;

import java.awt.Desktop;
import org.w3c.dom.Element;

import java.io.*;
import java.util.Date;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.jeffpiazza.derby.devices.SimulatedDevice;

// TODO: Suppress heartbeats with uninteresting responses
public class LogWriter implements HttpTask.MessageTracer {
  public static class FileAndPrintWriter {
    public FileAndPrintWriter(File file, PrintWriter writer) {
      this.file = file;
      this.writer = writer;
    }
    public File file;
    public PrintWriter writer;
  }
  private FileAndPrintWriter writer;

  public LogWriter() {
    try {
      this.writer = LogFileFactory.makeLogFile();
    } catch (IOException ex) {
      System.err.println("Unable to create log file");
      Logger.getLogger(LogWriter.class.getName()).log(Level.SEVERE, null, ex);
    }
    serialPortLog(INTERNAL, "Started at " + Timestamp.string());
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
          } catch (InterruptedException ex) {
          }
          serialPortLog(INTERNAL, Timestamp.string());
        }
      }
    }).start();
  }

  public static final int INCOMING = 0;
  public static final int OUTGOING = 1;
  public static final int INTERNAL = 2;

  private static final char HTTP_LOG = 'H';
  private static final char SERIAL_PORT_LOG = 'S';
  private static final char SIMULATION_LOG = '*';

  public void serialPortLog(int direction, String msg) {
    if (writer != null) {
      writer.writer.println("+" + Timestamp.brief() + SERIAL_PORT_LOG + "\t\t" +
                     (direction == INCOMING ? "<-- " :
                      direction == OUTGOING ? "--> " :
                         "INT ") +
                     msg.replace("\r", "\\r"));
    }
  }

  public void serialPortLogInternal(String msg) {
    serialPortLog(INTERNAL, msg);
  }

  public void httpLog(int direction, String msg) {
    if (writer != null) {
      writer.writer.println("+" + Timestamp.brief()+ HTTP_LOG + "\t\t\t" +
                     (direction == INCOMING ? "<-- " :
                      direction == OUTGOING ? "--> " :
                         "INT ") +
                     msg.replace("\r", "\\r"));
    }
  }

  public void simulationLog(String msg) {
    System.out.println();
    System.out.println(msg);
    if (writer != null) {
      writer.writer.println("\n+" + Timestamp.brief()+ SIMULATION_LOG + "\t" + msg);
    }
  }

  public void onMessageSend(Message m, String params) {
    httpLog(OUTGOING, m.asParameters());
  }

  public void onMessageResponse(Message m, Element response) {
    httpLog(INCOMING, XmlSerializer.serialized(response));
  }

  public void traceInternal(String s) {
    httpLog(INTERNAL, s);
  }

  public void stacktrace(Throwable t) {
    if (writer != null) {
      t.printStackTrace(writer.writer);
    }
  }

  public void showLogFile() {
    try {
      Desktop.getDesktop().open(writer.file.getParentFile());
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
