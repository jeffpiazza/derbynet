package org.jeffpiazza.derby.serialport;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.HashMap;
import jssc.SerialPortException;
import org.jeffpiazza.derby.LogWriter;

// For testing a new TimerDevice class, this simulates the data arriving on
// the serial port.
public class PlaybackSerialPortWrapper extends EventDrivenPortWrapper {
  private static String[] program;
  private HashMap<String, Integer> commands_responses =
      new HashMap<String, Integer>();
  private int program_counter = 0;
  private ArrayDeque<String> messages = new ArrayDeque<String>();
  private long pausedUntil = -1L;

  public static void setFilename(String filename) {
    try {
      BufferedReader reader = new BufferedReader(new FileReader(filename));
      ArrayList<String> lines = new ArrayList<String>();
      try {
        String line;
        while ((line = reader.readLine()) != null) {
          if ((line.length() > 0) && (line.charAt(0) == '#')) {
            line = line.trim();
            if (line.startsWith("##")) {
              continue;
            }
          } else {
            line = line + "\r\n";
          }
          lines.add(line);
        }
      } finally {
        reader.close();
      }
      program = (String[]) lines.toArray(new String[lines.size()]);
    } catch (Throwable t) {
      LogWriter.stacktrace(t);
      System.exit(1);
    }
  }

  public PlaybackSerialPortWrapper()
      throws SerialPortException {
    System.out.println("Creating simulated port wrapper");
    pushMessages();
    new Thread() {
      public void run() {
        try {
          for (;;) {
            Thread.sleep(1000L);
            noticeContact();
            pushMessages();
          }
        } catch (Throwable t) {
        }
      }
    }.start();
  }

  private void pushMessages() {
    try {
      onPortDataPending();
    } catch (SerialPortException ex) {
    }
  }

  private int plus1(int pc) {
    ++pc;
    if (pc >= program.length) {
      pc = 0;
    }
    return pc;
  }

  private int interpretOne(int pc) {
    String line = program[pc];
    System.out.println("At " + pc + " interpret: " + line.trim());
    if ((line.length() > 0) && (line.charAt(0) == '#')) {
      if (line.startsWith("#on ")) {
        commands_responses.put(line.substring(4), Integer.valueOf(pc + 1));
        while (!program[(++pc)].equals("#end")) {
        }
        return plus1(pc);
      }
      if (line.equals("#end")) {
        messages.addLast("");
        pushMessages();
        return -1;
      }
      if (line.equals("#pause")) {
        pausedUntil = (System.currentTimeMillis() + 5000L);
        return plus1(pc);
      }
      System.err.println("Unrecognized program marker: " + line);
      System.exit(1);
      return -1;
    }
    messages.addLast(line);
    return plus1(pc);
  }

  public void writeStringToPort(String s) throws SerialPortException {
    Integer response = commands_responses.get(s);
    if (response != null) {
      System.out.println(
          "Response for [[" + s + "]] is " + response);
      int pc = response.intValue();
      while (pc >= 0) {
        pc = interpretOne(pc);
      }
    } else {
      System.out.println("* No response for [[" + s + "]]");
    }
  }

  protected String readStringFromPort() throws SerialPortException {
    while (messages.size() == 0 && pausedUntil < System.currentTimeMillis()) {
      program_counter = interpretOne(program_counter);
      if (program_counter < 0) {
        System.err.println("Playback program isn't supposed to terminate!");
        System.exit(1);
      }
    }
    if (messages.size() == 0) {
      System.out.println("(Paused)");
      return "";
    }
    return (String) messages.removeFirst();
  }

  public boolean setPortParams(int baudRate, int dataBits, int stopBits,
                               int parity, boolean setRTS, boolean setDTR)
      throws SerialPortException {
    return true;
  }

  public void abandon() throws SerialPortException {
  }

  public void close() throws SerialPortException {
  }

  public void setDtr(boolean enabled) throws SerialPortException {
  }

  public String getPortName() {
    return "Simulated Port";
  }
}
