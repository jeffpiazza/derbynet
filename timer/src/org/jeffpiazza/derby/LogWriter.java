package org.jeffpiazza.derby;

import org.w3c.dom.Element;

import java.io.*;
import java.util.Date;
import java.util.logging.Level;
import java.util.logging.Logger;

// TODO: Suppress heartbeats with uninteresting responses
public class LogWriter implements HttpTask.MessageTracer {
  private PrintWriter writer;

  public LogWriter() {
    try {
      this.writer = LogFileFactory.makeLogFile();
    } catch (IOException ex) {
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

  public void serialPortLog(int direction, String msg) {
    if (writer != null) {
      writer.println("+" + Timestamp.brief() + "\t\t" +
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
      writer.println("+" + Timestamp.brief()+ "\t\t\t" +
                     (direction == INCOMING ? "<-- " :
                      direction == OUTGOING ? "--> " :
                         "INT ") +
                     msg.replace("\r", "\\r"));
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
}
