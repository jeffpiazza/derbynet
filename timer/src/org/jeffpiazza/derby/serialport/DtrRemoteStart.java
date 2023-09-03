package org.jeffpiazza.derby.serialport;

import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;
import org.jeffpiazza.derby.devices.RemoteStartInterface;

public class DtrRemoteStart implements RemoteStartInterface {
  private TimerPortWrapper portWrapper;

  public DtrRemoteStart(TimerPortWrapper port_wrapper) {
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
