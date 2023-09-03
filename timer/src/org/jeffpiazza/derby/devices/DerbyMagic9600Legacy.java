package org.jeffpiazza.derby.devices;

import jssc.SerialPort;
import jssc.SerialPortException;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;

public class DerbyMagic9600Legacy extends DerbyMagicLegacy {
  public DerbyMagic9600Legacy(TimerPortWrapper portWrapper) {
    super(portWrapper);
  }

  @Override
  public boolean canBeIdentified() {
    return false;
  }

  public static String toHumanString() {
    return "Derby Magic timer (force 9600 baud)";
  }

  @Override
  public boolean probe() throws SerialPortException {
    return probeAtSpeed(SerialPort.BAUDRATE_9600);
  }

}
