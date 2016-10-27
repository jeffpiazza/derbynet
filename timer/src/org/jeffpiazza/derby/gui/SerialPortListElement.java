package org.jeffpiazza.derby.gui;

import jssc.SerialPort;

public class SerialPortListElement {
  private SerialPort port;
  private boolean wontOpen;

  public SerialPortListElement(SerialPort port) {
    this.port = port;
    this.wontOpen = false;
  }

  public boolean wontOpen() {
    return wontOpen;
  }

  public void setWontOpen(boolean value) {
    wontOpen = value;
  }

  public SerialPort port() {
    return port;
  }

  public String toString() {
    return port.getPortName();
  }
}
