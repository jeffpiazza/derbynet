/* This "port wrapper" starts a subprocess and reads and writes to it, rather
   than a serial port.
 */
package org.jeffpiazza.derby.serialport;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.Arrays;
import java.util.List;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;

public class SubprocessPortWrapper extends TimerPortWrapper {
  Process subproc;
  Reader reader;

  public SubprocessPortWrapper(List<String> command) throws IOException,
                                                            SerialPortException {
    ProcessBuilder pb = new ProcessBuilder(command);
    pb.redirectErrorStream(true);
    subproc = pb.start();
    reader = new InputStreamReader(subproc.getInputStream());
    (new Thread() {
      @Override
      public void run() {
        StringBuilder builder = new StringBuilder();
        try {
          int ch;
          while ((ch = reader.read()) >= 0) {
            builder.append((char) ch);
            if (!reader.ready()) {
              String s = builder.toString();
              builder.setLength(0);
              acceptReadData(s);
              noticeContact();
            }
          }
        } catch (Exception ex) {
          System.err.println("Exception from reader thread: " + ex.getMessage());
        }
      }

    }).start();
  }

  @Override
  public boolean setPortParams(int baudRate, int dataBits, int stopBits,
                               int parity, boolean setRTS, boolean setDTR)
      throws SerialPortException {
    return true;
  }

  @Override
  public void setDtr(boolean enabled) throws SerialPortException {
  }

  @Override
  public void abandon() throws SerialPortException {
  }

  @Override
  public void close() throws SerialPortException {
  }

  @Override
  public String getPortName() {
    return "Subprocess";
  }

  @Override
  protected void writeStringToPort(String s) throws SerialPortException {
    try {
      subproc.getOutputStream().write((s + "\r\n").getBytes());
      subproc.getOutputStream().flush();
    } catch (IOException ex) {
      LogWriter.debugMsg("writeStringToPort: " + ex.toString());
    }
  }
}
