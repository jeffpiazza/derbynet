package org.jeffpiazza.derby;

import org.w3c.dom.Element;

import java.io.*;

// TODO: Suppress heartbeats with uninteresting responses
public class LogWriter implements HttpTask.MessageTracer {
  private PrintWriter writer;
  private long startTime = System.currentTimeMillis();

  public LogWriter() throws IOException {
    this.writer = LogFileFactory.makeLogFile();
    serialPortLog(INTERNAL, "Started at " + Timestamp.string());
  }

  public LogWriter(String path) throws IOException {
    this.writer = LogFileFactory.makeLogFile(path);
  }

  public static final int INCOMING = 0;
  public static final int OUTGOING = 1;
  public static final int INTERNAL = 2;

  public void serialPortLog(int direction, String msg) {
    writer.println("+" +
                   (System.currentTimeMillis() - startTime) +
                   "ms\t\t" +
                   (direction == INCOMING ? "<-- " :
                    direction == OUTGOING ? "--> " :
                       "INT ") +
                   msg.replace("\r", "\\r"));
  }

  public void serialPortLogInternal(String msg) {
    serialPortLog(INTERNAL, msg);
  }

  public void httpLog(int direction, String msg) {
    writer.println("+" +
                   (System.currentTimeMillis() - startTime)
                   + "ms\t\t\t" +
                   (direction == INCOMING ? "<-- " :
                    direction == OUTGOING ? "--> " :
                       "INT ") +
                   msg.replace("\r", "\\r"));
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
