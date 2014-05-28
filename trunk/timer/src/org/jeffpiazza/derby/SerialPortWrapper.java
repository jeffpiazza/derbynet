package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

// Usage:
//
// SerialPort port = new SerialPort(...);
// SerialPortWrapper wrapper = new SerialPortWrapper(port);
//
// In TimerDevice.probe():
// wrapper.port().setParams(SerialPort.BAUDRATE_9600, ...);
// wrapper.write(...), etc.
//
// wrapper.write(...), wrapper.writeAndWaitForResponse(...), wrapper.next(deadline), wrapper.next(), ...
//
// finally
// port.removeEventListener();
//

public class SerialPortWrapper implements SerialPortEventListener {
    private SerialPort port;
    private String leftover;
    private ArrayList<String> queue;  // Messages received from timer
    private PrintWriter logwriter;

    public interface Detector {
        // Return true if line has been handled, and therefore should
        // not be added to queue to be returned by next().
        boolean test(String line);
    }
    private ArrayList<Detector> detectors;

    public SerialPortWrapper(SerialPort port, PrintWriter logwriter) throws SerialPortException {
        this.port = port;
        this.leftover = "";
        this.queue = new ArrayList<String>();
	this.detectors = new ArrayList<Detector>();
	this.logwriter = logwriter;

        if (!port.purgePort(SerialPort.PURGE_RXCLEAR | SerialPort.PURGE_TXCLEAR)) {
            System.out.println("purgePort failed.");  // TODO
            // return false;
        }

        log(INTERNAL, "SerialPortWrapper attached");
        port.addEventListener(this, SerialPort.MASK_RXCHAR);
    }

    public SerialPortWrapper(SerialPort port) throws SerialPortException {
	this(port, new PrintWriter(System.out));
    }

    public SerialPort port() { return port; }

    public void registerDetector(Detector detector) {
        synchronized (detectors) {
            detectors.add(detector);
        }
    }

    public void unregisterDetector(Detector detector) {
        synchronized (detectors) {
            detectors.remove(detector);
        }
    }

    public static final int INCOMING = 0;
    public static final int OUTGOING = 1;
    public static final int INTERNAL = 2;

    public void log(int direction, String msg) {
        logwriter.println("\t\t" + System.currentTimeMillis() + " " +
                           (direction == INCOMING ? "<-- " :
                            direction == OUTGOING ? "--> " :
                            "INT ") +
                           msg.replace("\r", "\\r"));
    }

    // SerialPortEventListener interface: invoked 
    public void serialEvent(SerialPortEvent event) {
        try {
            if (event.isRXCHAR()) {
                read();
            } else {
                System.out.println("Event type is " + event.getEventType());
            }
        } catch (Exception e) {
            System.out.println("serialEvent gets an exception: " + e);
        }
    }

    // Process incoming characters from the device.  Whenever a full
    // newline-terminated line is formed, add it to the queue.  Any
    // remaining characters, forming an incomplete line, remain in
    // leftover.
    private void read() throws SerialPortException {
        try {
            while (true) {
                String s = port.readString();
                if (s == null || s.length() == 0)
                    break;
                s = leftover + s;
                int cr;
                while ((cr = s.indexOf('\n')) >= 0) {
                    String line = s.substring(0, cr).trim();
                    if (line.length() > 0) {
                        log(INCOMING, line);
                        boolean handled = false;
                        synchronized (detectors) {
                            for (Detector detector : detectors) {
                                if (detector.test(line)) {
                                    handled = true;
                                    break;
                                }
                            }
                        }
                        if (!handled) {
                            synchronized (queue) {
                                queue.add(line);
                            }
                        }
                    }
                    s = s.substring(cr + 1);
                }
                leftover = s;
                // log(INTERNAL, "leftover = <<" + leftover + ">>");
            }
        } catch (Exception exc) {
            System.out.println("Exception while reading: " + exc);
            exc.printStackTrace();  // TODO
        }
    }

    public void write(String s) throws SerialPortException {
        log(OUTGOING, s);
        port.writeString(s);
    }

    // These are unsatisfactory, because it's not a certainty that
    // the single next thing sent is the response we're looking
    // for.  But they at least ensure that there's SOME response between 
    public String writeAndWaitForResponse(String cmd) throws SerialPortException {
        return writeAndWaitForResponse(cmd, 2000);
    }

    public String writeAndWaitForResponse(String cmd, int timeout) throws SerialPortException {
        write(cmd);
        return next(System.currentTimeMillis() + timeout);
    }

    public boolean hasAvailable() {
        synchronized (queue) {
            return queue.size() > 0;
        }
    }

    public String next(long deadline) {
        while (System.currentTimeMillis() < deadline) {
            if (hasAvailable()) {
                return next();
            } else {
                try {
                    Thread.sleep(50);  // Sleep briefly, 50ms = 0.05s
                } catch (Exception exc) {}
            }
        }
        return null;
    }

    public String next() {
        String s = null;
        synchronized (queue) {
            if (queue.size() > 0) {
                s = queue.remove(0);
            }
        }
        if (s != null && s.length() > 0 && s.charAt(0) == '@') {
            System.out.println("* Spurious '@' removed");  // TODO
            s = s.substring(1);
        }
        return s;
    }
}
