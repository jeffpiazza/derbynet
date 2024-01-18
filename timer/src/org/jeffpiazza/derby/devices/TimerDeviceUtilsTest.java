package org.jeffpiazza.derby.devices;

import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import jssc.SerialPortException;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;

public class TimerDeviceUtilsTest {
  private static class MockDetector implements TimerPortWrapper.Detector {
    @Override
    public String apply(String line) throws SerialPortException {
      if (expectations.isEmpty()) {
        System.err.println("***** FAIL: Inner detected called on \"" + line
            + "\" with no expectations set.");
      } else {
        expectEq(line, expectations.get(0));
        expectations.remove(0);
      }
      if (line.length() < 5) {
        return "";
      }
      return line.substring(5);
    }

    private List<String> expectations = new ArrayList<String>();

    public void expect(String s) {
      expectations.add(s);
    }

    public void done() {
      while (!expectations.isEmpty()) {
        System.err.println("**** FAIL: Unmet expectation: \""
            + expectations.get(0) + "\"");
        expectations.remove(0);
      }
    }
  }

  private static void expectEq(String s1, String s2) {
    if (!s1.equals(s2)) {
      System.err.println("**** FAIL: \"" + s1 + "\" != \"" + s2 + "\"");
    }
  }

  public static void main(String[] args) {
    try {
      MockDetector mock = new MockDetector();
      TimerPortWrapper.Detector detector
          = new TimerDeviceUtils.SplittingDetector(
              mock);

      System.out.println("Empty string test");
      mock.expect("");
      expectEq(detector.apply(""), "");
      mock.done();

      System.out.println("Single segment test");
      mock.expect("0123456789");
      expectEq(detector.apply("0123456789"), "56789");
      mock.done();

      System.out.println("Three-segment test");
      mock.expect("0123");
      mock.expect("abcdefghijklm");
      mock.expect("ABCDEFGHI");
      expectEq(detector.apply("0123\n\rabcdefghijklm\r\n\nABCDEFGHI"),
               "\n\rfghijklm\r\n\nFGHI");
      mock.done();

      System.out.println("All newlines test");
      mock.expect("");
      mock.expect("");
      expectEq(detector.apply("\r\n\r\n\r\n"),
               "\r\n\r\n\r\n");
      mock.done();
    } catch (SerialPortException ex) {
      Logger.getLogger(TimerDeviceUtilsTest.class.getName()).
          log(Level.SEVERE, null, ex);
    }
  }
}
