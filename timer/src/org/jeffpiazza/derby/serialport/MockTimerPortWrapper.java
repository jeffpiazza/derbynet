package org.jeffpiazza.derby.serialport;

import org.jeffpiazza.derby.serialport.TimerPortWrapper;
import jssc.SerialPortException;

public class MockTimerPortWrapper extends TimerPortWrapper {
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
    return "MockSerialPort";
  }

  @Override
  protected void writeStringToPort(String s) throws SerialPortException {
  }
}
