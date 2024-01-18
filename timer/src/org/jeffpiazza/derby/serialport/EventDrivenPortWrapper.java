/*
 */

package org.jeffpiazza.derby.serialport;

import jssc.SerialPortException;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;

public abstract class EventDrivenPortWrapper extends TimerPortWrapper {
  protected void onPortDataPending() throws SerialPortException {
    try {
      String s;
      while ((s = readStringFromPort()) != null && s.length() > 0) {
        if (Flag.debug_io.value()) {
          LogWriter.debugMsg("read(" + describeString(s) + ")");
        }
        acceptReadData(s);
      }
    } catch (Exception exc) {
      LogWriter.stacktrace(exc);
      System.err.println("Exception while reading: " + exc);
      exc.printStackTrace();
    }
  }

  // Read a string from the port.
  protected abstract String readStringFromPort() throws SerialPortException;
}
