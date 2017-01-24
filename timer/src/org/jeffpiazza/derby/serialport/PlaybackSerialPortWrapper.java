package org.jeffpiazza.derby.serialport;

import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;

// For testing a new TimerDevice class, this simulates the data arriving on
// the serial port.
public class PlaybackSerialPortWrapper extends SerialPortWrapper {
  public PlaybackSerialPortWrapper(LogWriter logwriter)
      throws SerialPortException {
    super(null, logwriter);
    System.out.println("Creating simulated port wrapper");
    // Without a real serial port, nothing will generate events to cause a
    // read() to happen, so we have to arrange it ourselves.
    (new Thread() {
      @Override
      public void run() {
        while (true) {
          try {
            Thread.sleep(1000);
            read();
          } catch (Throwable ex) {
          }
        }
      }
    }).start();
  }

  private int counter = 0;

  private String[] messages = {
    "2 2.3374  3 2.7491  4 3.0885  1 4.2156\r\n",
    "1 2.8838  2 3.4474  3 3.9291  4 4.4491\r\n",
    ""};

  @Override
  protected String readStringFromPort() throws SerialPortException {
    if (counter >= messages.length) { counter = 0; }
    return messages[counter++];
  }

  @Override
  public boolean setPortParams(int baudRate, int dataBits, int stopBits, int parity,
                           boolean setRTS, boolean setDTR)
      throws SerialPortException {
    return true;
  }

  @Override
  public void closePort() throws SerialPortException {
  }

  @Override
  public void writeStringToPort(String s) throws SerialPortException {
    // Nothing to do!
  }
}
