package org.jeffpiazza.derby.devices;

import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public class DerbyMagic9600 extends DerbyMagicDevice {
  public DerbyMagic9600(SerialPortWrapper portWrapper) {
    super(portWrapper);
  }

  @Override
  public boolean probe() throws SerialPortException {
    return probeAtSpeed(SerialPort.BAUDRATE_9600);
  }

}
