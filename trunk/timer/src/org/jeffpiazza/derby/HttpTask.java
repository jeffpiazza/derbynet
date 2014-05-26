package org.jeffpiazza.derby;

import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;
import java.net.URLEncoder;

public class HttpTask implements Runnable {
    private ClientSession session;

    public HttpTask(String base_url, String username, String password) throws IOException {
        session = new ClientSession(base_url, username, password);
    }

    public void send(Message message) throws IOException {
        session.sendTimerMessage(message.asParameters());
    }

    public interface HeatReadyCallback {
        void heatReady(int laneMask);
    }
    public void registerHeatReadyCallback(HeatReadyCallback cb) {
        // TODO
    }

    // HttpTask has a queue for events to send, registers callbacks for HEAT-READY(with lane mask) and ABORT
    // Continually checks queue, sending queued events; otherwise sends a HEARTBEAT and sleeps a known amount of time.
    public void run() {
        // TODO
    }
}
