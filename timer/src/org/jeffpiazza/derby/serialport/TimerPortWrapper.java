/*
 */

package org.jeffpiazza.derby.serialport;

import java.util.ArrayList;
import java.util.List;
import jssc.SerialPortException;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.devices.TimerDevice;

public abstract class TimerPortWrapper {
  public static final int COMMAND_DRAIN_MS = 100;
  // Number of milliseconds before considering connection lost
  protected static final long LOST_CONTACT_THRESHOLD = 2000;

  // A Detector looks for asynchronously-provided data in the data stream.
  // Ordinary Detectors get applied to a complete newly-received line of data
  // before adding that line to the queue.  SerialPortWrapper also supports a
  // single "early" detector that runs on an incomplete line, every time data
  // is added to the line.
  public interface Detector {
    // Return that part of line not handled by this detector
    String apply(String line) throws SerialPortException;

    public static String applyDetectors(String line, List<Detector> detectors)
        throws SerialPortException {
      line = line.trim();
      if (detectors != null) {
        boolean match_more = (line.length() > 0);
        while (match_more) {
          match_more = false;
          for (Detector d : detectors) {
            String s2 = d.apply(line);
            if (line != s2) {  // Intentional pointer comparison
              line = s2;
              match_more = (line.length() > 0);
              break;
            }
          }
        }
      }
      return line;
    }
  }

  protected static String describeString(String s) {
    for (int i = s.length() - 1; i >= 0; --i) {
      int c = s.codePointAt(i);
      if (20 <= c && c < 127) {
      } else if (c == 10) {
        s = s.substring(0, i) + "\\n" + s.substring(i + 1);
      } else if (c == 9) {
        s = s.substring(0, i) + "\\t" + s.substring(i + 1);
      } else if (c == 13) {
        s = s.substring(0, i) + "\\r" + s.substring(i + 1);
      } else {
        s = s.substring(0, i) + "\\{" + c + "}" + s.substring(i + 1);
      }
    }
    return s;
  }
  // A sequence of end-of-line characters that should be written
  // at the end of each write().
  protected String end_of_line = "";
  // Received characters that don't yet make up a complete line, i.e., still
  // waiting for a newline character.
  protected String leftover;
  // System time in millis when leftover was last added to.
  protected long last_char_received;
  // Messages (full lines) received from timer
  protected ArrayList<String> queue;
  // System time in millis when we last sent a command to the serial port.
  // TODO What we really want is to track the last command for which we actually
  // expect a response.
  protected long last_command;
  // System time in millis when we last received an event from the serial
  // port; used to detect lost contact.
  protected long last_contact;
  // Latches true if any data has been received from the timer.
  protected boolean has_ever_spoken = false;
  protected final ArrayList<Detector> detectors = new ArrayList<Detector>();
  // If there is one, the early detector gets applied repeatedly each time the
  // buffer changes, without waiting for a newline character.
  protected Detector earlyDetector;

  public TimerPortWrapper() {
    this.leftover = "";
    this.queue = new ArrayList<String>();
  }

  // These xxxPortXxx methods are the only ones that interact directly with the
  // SerialPort member.
  public abstract boolean setPortParams(int baudRate, int dataBits, int stopBits,
                                        int parity, boolean setRTS,
                                        boolean setDTR) throws
                                                               SerialPortException;

  public boolean setPortParams(int baudRate, int dataBits, int stopBits,
                               int parity) throws SerialPortException {
    return setPortParams(baudRate, dataBits, stopBits, parity,
                         !Flag.clear_rts_dtr.value(),
                         !Flag.clear_rts_dtr.value());
  }

  public abstract void setDtr(boolean enabled) throws SerialPortException;

  public void setEndOfLine(String end_of_line) {
    this.end_of_line = end_of_line;
  }

  public abstract void abandon() throws SerialPortException;
  public abstract void close() throws SerialPortException;

  public abstract String getPortName();

  protected abstract void writeStringToPort(String s) throws SerialPortException;

  // TODO Are these thread-safe?  Do they need to be?
  public long millisSinceLastCommand() {
    return System.currentTimeMillis() - last_command;
  }

  public long millisSinceLastContact() {
    return System.currentTimeMillis() - last_contact;
  }

  public void checkConnection() throws TimerDevice.LostConnectionException {
    if (millisSinceLastContact() > LOST_CONTACT_THRESHOLD) {
      throw new TimerDevice.LostConnectionException();
    }
  }

  public void registerDetector(Detector detector) {
    synchronized (detectors) {
      detectors.add(detector);
    }
  }

  public void unregisterDetector(Detector detector) {
    synchronized (detectors) {
      detectors.remove(detector);
    }
  }

  // The early detector, if there is one, runs on the buffered string from the
  // timer device before the string is divided up into newline-separated lines.
  // It's "early" because it doesn't wait for a newline to arrive to mark the
  // end of a line.  Note that the string passed to the detector may contain
  // zero or more newlines.
  public void registerEarlyDetector(Detector detector) {
    synchronized (detectors) {
      earlyDetector = detector;
    }
  }

  public void noticeContact() {
    last_contact = System.currentTimeMillis();
  }

  // Process incoming characters from the device.  Whenever a full
  // newline-terminated line is formed, add it to the queue.  Any
  // remaining characters, forming an incomplete line, remain in
  // leftover.
  protected void acceptReadData(String s) throws SerialPortException {
    synchronized (leftover) {
      has_ever_spoken = true;
      s = leftover + s;
      synchronized (detectors) {
        if (earlyDetector != null) {
          s = earlyDetector.apply(s);
        }
      }
      int cr;
      while ((cr = s.indexOf('\n')) >= 0) {
        enqueueLine(s.substring(0, cr));
        s = s.substring(cr + 1);
      }
      leftover = s;
      last_char_received = System.currentTimeMillis();
    }
  }

  protected String applyDetectors(String line) {
    try {
      line = line.trim();
      if (line.length() > 0) {
        LogWriter.serialIn(line);
      }
      line = Detector.applyDetectors(line, detectors);
    } catch (SerialPortException exc) {
      LogWriter.stacktrace(exc);
      System.err.println("Exception while reading: " + exc);
      exc.printStackTrace();
      line = "";
    }
    return line;
  }

  protected void enqueueLine(String line) {
    line = applyDetectors(line);
    if (line.length() > 0) {
      synchronized (queue) {
        queue.add(line);
      }
    }
  }

  public void write(String s) throws SerialPortException {
    LogWriter.serialOut(s);
    last_command = System.currentTimeMillis();
    writeStringToPort(s + end_of_line);
  }

  // These are unsatisfactory, because it's not a certainty that
  // the single next thing sent is the response we're looking
  // for.  But they at least ensure that there's SOME response between
  public String writeAndWaitForResponse(String cmd) throws SerialPortException {
    return writeAndWaitForResponse(cmd, 2000);
  }

  public String writeAndWaitForResponse(String cmd, int timeout)
      throws SerialPortException {
    clear();
    write(cmd);
    return next(System.currentTimeMillis() + timeout);
  }

  public String next(long deadline) {
    while (System.currentTimeMillis() < deadline) {
      String s = nextNoWait();
      if (s != null) {
        return s;
      }
      try {
        Thread.sleep(50); // Sleep briefly, 50ms = 0.05s
      } catch (Exception exc) {
      }
    }
    return null;
  }

  public String nextNoWait() {
    String s = null;
    synchronized (queue) {
      if (queue.size() > 0) {
        s = queue.remove(0);
      }
    }
    if (s == null) {
      synchronized (leftover) {
        if (leftover.length() > 0 && Flag.newline_expected_ms.value() > 0 && System.currentTimeMillis() - last_char_received > Flag.newline_expected_ms.value()) {
          if (Flag.debug_io.value()) {
            LogWriter.debugMsg("infer newline(" + describeString(leftover) + ")");
          }
          // Don't call enqueueLine here, as we want the string to return now.
          s = applyDetectors(leftover);
          leftover = "";
          if (s.length() == 0) {
            s = null;
          }
        }
      }
    }
    if (s != null && s.length() > 0 && s.charAt(0) == '@') {
      s = s.substring(1);
    }
    return s;
  }

  // Empties the queue immediately
  public void clear() {
    synchronized (queue) {
      for (String s : queue) {
        LogWriter.serial("CLEARED: " + s);
      }
      queue.clear();
    }
  }

  // Waits until deadline (at the latest) for at least expectedLines input lines.
  // to appear in the queue, then drains the queue.
  public void drain(long deadline, int expectedLines) {
    while (System.currentTimeMillis() < deadline) {
      synchronized (queue) {
        if (queue.size() >= expectedLines) {
          if (Flag.mark_ignored_timer_responses.value()) {
            for (String s : queue) {
              LogWriter.serial("DRAINED: " + s);
            }
          }
          queue.subList(0, expectedLines).clear();
          return;
        }
      }
      try {
        Thread.sleep(50); // Sleep briefly, 50ms = 0.05s
      } catch (Exception exc) {
      }
    }
  }

  public void drain() {
    drain(System.currentTimeMillis() + 500, 1);
  }

  public void drainForMs(int ms) {
    long deadline = System.currentTimeMillis() + ms;
    while (next(deadline) != null) {
      ;
    }
  }

  public void drainForMs() {
    drainForMs(COMMAND_DRAIN_MS);
  }

  public boolean hasEverSpoken() {
    return has_ever_spoken;
  }

  public void setHasEverSpoken() {
    has_ever_spoken = true;
  }

  public void writeAndDrainResponse(String cmd, int expectedLines, int timeout)
      throws SerialPortException {
    clear();
    write(cmd);
    if (expectedLines > 0) {
      drain(System.currentTimeMillis() + timeout, expectedLines);
    }
  }

  public void writeAndDrainResponse(String cmd) throws SerialPortException {
    writeAndDrainResponse(cmd, 1, 2000);
  }
}
