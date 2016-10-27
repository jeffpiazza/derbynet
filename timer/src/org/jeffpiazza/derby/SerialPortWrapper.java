package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

// Usage:
//
// SerialPort port = new SerialPort(...);
// SerialPortWrapper wrapper = new SerialPortWrapper(port);
//
// In TimerDevice.probe():
// wrapper.port().setParams(SerialPort.BAUDRATE_9600, ...);
// wrapper.write(...), etc.
//
// wrapper.write(...), wrapper.writeAndWaitForResponse(...), wrapper.next(deadline), wrapper.next(), ...
//
// finally
// port.removeEventListener();
//
public class SerialPortWrapper implements SerialPortEventListener {
  private SerialPort port;
  // Received characters that don't yet make up a complete line, i.e., still
  // waiting for a newline character.
  private String leftover;
  // Messages (full lines) received from timer
  private ArrayList<String> queue;
  private LogWriter logwriter;
  // System time in millis when we last sent a command to the serial port.
  // TODO What we really want is to track the last command for which we actually
  // expect a response.
  private long last_command;
  // System time in millis when we last received a character from the serial
  // port; used to detect lost contact.
  private long last_contact;

  public interface Detector {
    // Return that part of line not handled by this detector
    String apply(String line);
  }
  private ArrayList<Detector> detectors;

  public SerialPortWrapper(SerialPort port, LogWriter logwriter) throws
      SerialPortException {
    this.port = port;
    this.leftover = "";
    this.queue = new ArrayList<String>();
    this.detectors = new ArrayList<Detector>();
    this.logwriter = logwriter;

    if (!port.purgePort(SerialPort.PURGE_RXCLEAR | SerialPort.PURGE_TXCLEAR)) {
      System.out.println("purgePort failed.");  // TODO
      // return false;
    }

    logwriter.serialPortLog(LogWriter.INTERNAL, "SerialPortWrapper attached");
    port.addEventListener(this, SerialPort.MASK_RXCHAR);
  }

  public SerialPort port() {
    return port;
  }

  public LogWriter logWriter() {
    return logwriter;
  }

  public long millisSinceLastCommand() {
    return System.currentTimeMillis() - last_command;
  }

  public long millisSinceLastContact() {
    return System.currentTimeMillis() - last_contact;
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

  // SerialPortEventListener interface: invoked 
  public void serialEvent(SerialPortEvent event) {
    try {
      if (event.isRXCHAR()) {
        last_contact = System.currentTimeMillis();
        read();
      } else {
        System.out.println("Event type is " + event.getEventType());
      }
    } catch (Exception e) {
      System.out.println("serialEvent gets an exception: " + e);
    }
  }

  // Process incoming characters from the device.  Whenever a full
  // newline-terminated line is formed, add it to the queue.  Any
  // remaining characters, forming an incomplete line, remain in
  // leftover.
  private void read() throws SerialPortException {
    try {
      while (true) {
        String s = port.readString();
        if (s == null || s.length() == 0) {
          break;
        }
        s = leftover + s;
        int cr;
        while ((cr = s.indexOf('\n')) >= 0) {
          String line = s.substring(0, cr).trim();
          if (line.length() > 0) {
            logwriter.serialPortLog(LogWriter.INCOMING, line);
            synchronized (detectors) {
              for (Detector detector : detectors) {
                line = detector.apply(line);
                if (line.length() == 0) {
                  break;
                }
              }
            }
          }
          if (line.length() > 0) {
            synchronized (queue) {
              queue.add(line);
            }
          }

          s = s.substring(cr + 1);
        }
        leftover = s;
        // logwriter.serialPortLog(LogWriter.INTERNAL, "leftover = <<" + leftover + ">>");
      }
    } catch (Exception exc) {
      System.out.println("Exception while reading: " + exc);
      exc.printStackTrace();  // TODO
    }
  }

  public void write(String s) throws SerialPortException {
    logwriter.serialPortLog(LogWriter.OUTGOING, s);
    last_command = System.currentTimeMillis();
    port.writeString(s);
  }

  // These are unsatisfactory, because it's not a certainty that
  // the single next thing sent is the response we're looking
  // for.  But they at least ensure that there's SOME response between 
  public String writeAndWaitForResponse(String cmd) throws SerialPortException {
    return writeAndWaitForResponse(cmd, 2000);
  }

  public String writeAndWaitForResponse(String cmd, int timeout) throws
      SerialPortException {
    clear();
    write(cmd);
    return next(System.currentTimeMillis() + timeout);
  }

  public boolean hasAvailable() {
    return hasAvailable(1);
  }

  public boolean hasAvailable(int expected) {
    synchronized (queue) {
      return queue.size() >= expected;
    }
  }

  public String next() {
    return next(System.currentTimeMillis() + 500);
  }

  public String next(long deadline) {
    while (System.currentTimeMillis() < deadline) {
      if (hasAvailable()) {
        return nextNoWait();
      } else {
        try {
          Thread.sleep(50);  // Sleep briefly, 50ms = 0.05s
        } catch (Exception exc) {
        }
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
    if (s != null && s.length() > 0 && s.charAt(0) == '@') {
      // System.out.println("* Spurious '@' removed");
      s = s.substring(1);
    }
    return s;
  }

  // Empties the queue immediately
  public void clear() {
    synchronized (queue) {
      queue.clear();
    }
  }

  // Waits until deadline (at the latest) for at least one expected input lines
  // to appear in the queue, then drains the queue.
  public void drain(long deadline, int expected) {
    while (System.currentTimeMillis() < deadline) {
      if (hasAvailable(expected)) {
        clear();
        return;
      } else {
        try {
          Thread.sleep(50);  // Sleep briefly, 50ms = 0.05s
        } catch (Exception exc) {
        }
      }
    }
  }

  public void drain() {
    drain(System.currentTimeMillis() + 500, 1);
  }

  public void writeAndDrainResponse(String cmd, int expected, int timeout)
      throws SerialPortException {
    clear();
    write(cmd);
    drain(System.currentTimeMillis() + timeout, expected);
  }

  public void writeAndDrainResponse(String cmd) throws SerialPortException {
    writeAndDrainResponse(cmd, 1, 2000);
  }
}
