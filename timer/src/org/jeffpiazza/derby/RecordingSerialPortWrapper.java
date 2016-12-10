package org.jeffpiazza.derby;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import jssc.SerialPort;
import jssc.SerialPortException;

public class RecordingSerialPortWrapper extends SerialPortWrapper {
  public RecordingSerialPortWrapper(SerialPort port, LogWriter logwriter)
      throws SerialPortException {
    super(port, logwriter);
    (new Thread() {
      @Override
      public void run() {
        while (true) {
          try {
            Thread.sleep(60000);
          } catch (InterruptedException ex) {
          }
          maybeDumpRecordings();
        }
      }
    }).start();
  }

  Map<String, ArrayList<String>> recorded =
      new HashMap<String, ArrayList<String>>();
  String lastWritten = "";
  boolean changed = false;

  @Override
  public void write(String s) throws SerialPortException {
    lastWritten = s;
    super.write(s);
  }

  @Override
  public String nextNoWait() {
    String read = super.nextNoWait();

    if (read != null) {
      ArrayList<String> values = recorded.get(lastWritten);
      if (values == null) {
        recorded.put(lastWritten, values = new ArrayList<String>());
      }
      if (!values.contains(read)) {
        values.add(read);
        changed = true;
      }
    }

    return read;
  }

  public void maybeDumpRecordings() {
    if (changed) {
      dumpRecordings();
      changed = false;
    }
  }

  public void dumpRecordings() {
    logWriter().traceInternal("Recorded transactions:");
    for (String key : recorded.keySet()) {
      for (String v : recorded.get(key)) {
        logWriter().traceInternal("\"" + key + "\"\t-> \"" + v + "\"");
      }
    }
  }
}
