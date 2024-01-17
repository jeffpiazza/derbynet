package org.jeffpiazza.derby.serialport;

import jssc.SerialPort;
import jssc.SerialPortEvent;
import jssc.SerialPortEventListener;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;

// Usage:
//
// SerialPort port = new SerialPort(...);
// SerialPortWrapper wrapper = new SerialPortWrapper(port);
//
// In TimerDevice.probe():
// wrapper.port().setParams(SerialPort.BAUDRATE_9600, ...);
// wrapper.write(...), etc.
//
// wrapper.write(...),
// wrapper.writeAndWaitForResponse(...),
// wrapper.next(deadline),
// wrapper.next(), ...
//
// finally
// port.removeEventListener();
//
public class SerialPortWrapper extends EventDrivenPortWrapper
    implements SerialPortEventListener {
  private SerialPort port;

  public SerialPortWrapper(SerialPort port) throws
      SerialPortException {
    this.port = port;

    if (port != null) {
      if (!port.purgePort(SerialPort.PURGE_RXCLEAR
          | SerialPort.PURGE_TXCLEAR)) {
        LogWriter.serial("purgePort failed.");
        System.err.println("purgePort failed.");
        // return false;
      }

      LogWriter.serial("SerialPortWrapper attached");
      port.addEventListener(this, SerialPort.MASK_RXCHAR);
    }
  }

  // These xxxPortXxx methods are the only ones that interact directly with the
  // SerialPort member.
  public boolean setPortParams(int baudRate, int dataBits, int stopBits,
                               int parity, boolean setRTS, boolean setDTR)
      throws SerialPortException {
    return port.setParams(baudRate, dataBits, stopBits, parity, setRTS, setDTR);
  }

  public void setDtr(boolean enabled) throws SerialPortException {
    port.setDTR(enabled);
  }

  public void abandon() throws SerialPortException {
    port.removeEventListener();
  }

  public void close() throws SerialPortException {
    port.closePort();
  }

  public String getPortName() {
    if (port == null) {
      return "SimulatedPort";
    }
    return port.getPortName();
  }


  // Read a string from the port.
  protected String readStringFromPort() throws SerialPortException {
    return port.readString();
  }

  protected void writeStringToPort(String s) throws SerialPortException {
    port.writeString(s);
  }

  // SerialPortEventListener interface: invoked
  public void serialEvent(SerialPortEvent event) {
    try {
      if (event.isRXCHAR()) {
        noticeContact();
        onPortDataPending();
      } else if (event.isTXEMPTY()) {
        // Not sure what this means or what causes it, but
        // it comes up sometimes on Windows
        noticeContact();
      } else {
        String msg = "\n(Unexpected serialEvent type: "
            + event.getEventType() + " with value " + event.getEventValue() + ")";
        LogWriter.info(msg);
        System.err.println(msg);
      }
    } catch (Exception e) {
      LogWriter.stacktrace(e);
      System.err.println("serialEvent gets an exception: " + e);
    }
  }
}
