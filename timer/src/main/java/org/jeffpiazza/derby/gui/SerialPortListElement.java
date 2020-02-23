package org.jeffpiazza.derby.gui;

public class SerialPortListElement {
  private String portName;
  private boolean wontOpen;

  public SerialPortListElement(String portName) {
    this.portName = portName;
    this.wontOpen = false;
  }

  public boolean wontOpen() {
    return wontOpen;
  }

  public void setWontOpen(boolean value) {
    wontOpen = value;
  }

  public String portName() {
    return portName;
  }

  public String toString() {
    return portName();
  }
}
