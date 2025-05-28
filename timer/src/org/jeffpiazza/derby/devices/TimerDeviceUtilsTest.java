package org.jeffpiazza.derby.devices;

import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import jssc.SerialPortException;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;

// java -cp timer/build:lib/jssc.jar org.jeffpiazza.derby.devices.TimerDeviceUtilsTest
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

        // 3A5A\r	Lane 1 enabled       Binary: 000001
        // 3C5A\r	Lanes 1-2 enabled    Binary: 000011
        // 3G5A\r	Lanes 1-3 enabled    Binary: 000111
        // 3O5A\r	Lanes 1-4 enabled    Binary: 001111
        // 3_5A\r	Lanes 1-5 enabled    Binary: 011111
        // 3|5A\r	Lanes 3, 4, 5, 6     Binary: 111100

        // .embedded_mask_command(/* "3@5A" as an int: */0x33403541, 16, 4)

      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x33403541, 16, 4, 0b0000001), "3A5A");
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x33403541, 16, 4, 0b0000011), "3C5A");
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x33403541, 16, 4, 0b0000111), "3G5A");
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x33403541, 16, 4, 0b0001111), "3O5A");
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x33403541, 16, 4, 0b0111100), "3|5A");
        //     cmd (hex) (ascii) Timeout
        //     32 a4 0d    2¤.    25
        //     32 a0 0d    2 .    24
        //     32 3f 0d    2?.    21-23 // Not sure why these repeat
        ///////  Should be 32 9c 0d for 23
        ///////  Should be 32 98 0d for 22
        ///////  Should be 32 94 0d for 21
        //     32 90 0d    2.    20
        //     32 3f 0d    2?.    16-19 // Not sure why these repeat
        //     32 7c 0d    2|.    15
        //     32 78 0d    2x.    14
        //     32 74 0d    2t.    13
        //     32 70 0d    2p.    12
        //     32 6c 0d    2l.    11
        //     32 68 0d    2h.    10
        //     32 40 0d    2@.    NO Timeout
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x3240, 2, 2, 24), "2\u00a0");
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x3240, 2, 2, 21), "2\u0094");
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x3240, 2, 2, 17), "2\u0084");
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x3240, 2, 2, 16), "2\u0080");

      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x3240, 2, 2, 15), "2\u007c");
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x3240, 2, 2, 14), "2x");
      expectEq(TimerDeviceUtils.embeddedFieldCommand(0x3240, 2, 2, 10), "2h");

     } catch (SerialPortException ex) {
      Logger.getLogger(TimerDeviceUtilsTest.class.getName()).
          log(Level.SEVERE, null, ex);
    }
  }
}
