package org.jeffpiazza.derby;

import org.w3c.dom.Element;

import java.io.*;

// TODO: Suppress heartbeats with uninteresting responses
public class LogWriter implements HttpTask.MessageTracer {
  private PrintWriter writer;

  public LogWriter() throws IOException {
    this.writer = LogFileFactory.makeLogFile();
  }

  public LogWriter(String path) throws IOException {
    this.writer = LogFileFactory.makeLogFile(path);
  }

  public static final int INCOMING = 0;
  public static final int OUTGOING = 1;
  public static final int INTERNAL = 2;

  public void serialPortLog(int direction, String msg) {
    writer.println(System.currentTimeMillis() + "\t\t" +
                   (direction == INCOMING ? "<-- " :
                    direction == OUTGOING ? "--> " :
                       "INT ") +
                   msg.replace("\r", "\\r"));
  }

  public void serialPortLogInternal(String msg) {
    serialPortLog(INTERNAL, msg);
  }

  public void httpLog(int direction, String msg) {
    writer.println(System.currentTimeMillis() + "\t\t\t" +
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
