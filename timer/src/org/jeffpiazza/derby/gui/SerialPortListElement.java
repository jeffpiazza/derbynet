package org.jeffpiazza.derby.gui;

import jssc.SerialPort;

public class SerialPortListElement {
  private SerialPort port;

  public SerialPortListElement(SerialPort port) {
    this.port = port;
  }

  public SerialPort port() {
    return port;
  }

  public String toString() {
    return port.getPortName();
  }
}
