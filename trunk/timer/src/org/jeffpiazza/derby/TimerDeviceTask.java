package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

// Periodic task for polling the TimerDevice
public class TimerDeviceTask implements Runnable {
    TimerDevice device;

    public TimerDeviceTask(TimerDevice device) {
        this.device = device;
    }

    public void run() {
        while (true) {
            try {
                device.poll();
            } catch (Throwable t) {
                t.printStackTrace();
            }
                
            synchronized (this) {
                try {
                    this.wait(1000); // in ms.
                } catch (Throwable t) {
                }
            }
        }
    }
}
