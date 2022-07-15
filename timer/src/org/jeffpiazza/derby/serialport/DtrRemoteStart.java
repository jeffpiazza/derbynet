package org.jeffpiazza.derby.serialport;

import java.util.logging.Level;
import java.util.logging.Logger;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.devices.RemoteStartInterface;

public class DtrRemoteStart implements RemoteStartInterface {
  private SerialPortWrapper portWrapper;

  public DtrRemoteStart(SerialPortWrapper port_wrapper) {
    this.portWrapper = port_wrapper;
  }

  @Override
  public boolean hasRemoteStart() {
    return true;
  }

  @Override
  public void remoteStart() throws SerialPortException {
    (new Thread() {
      @Override
      public void run() {
        try {
          portWrapper.setDtr(true);
          try { Thread.sleep(500); } catch (InterruptedException ex) {}
          portWrapper.setDtr(false);
        } catch (SerialPortException spe) {
          LogWriter.serial(spe.toString());
        }
      }
    }).start();
  }
}
