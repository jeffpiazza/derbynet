package org.jeffpiazza.derby;

import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;
import java.net.URLEncoder;

public class HttpTask implements Runnable {
    private ClientSession session;

    // TODO: Move these to ClientSession?  Somewhere else?

    // TODO: Heartbeat should supply whatever ancillary information
    // the timer supports (reset button pressed, lane blocked, etc.)

    // TODO: FinishedMessage should provide for place information, too.

    public interface Message {
        public void send(ClientSession session) throws IOException;
    }

    public static class HelloMessage implements Message {
        private int nlanes;
        public HelloMessage(int nlanes) {
            this.nlanes = nlanes;
        }

        public void send(ClientSession session) throws IOException {
            session.sendTimerMessage("message=HELLO&nlanes=" + nlanes);
        }
    }

    public static class StartedMessage implements Message {
        public void send(ClientSession session) throws IOException {
            session.sendTimerMessage("message=STARTED");
        }
    }

    public static class FinishedMessage implements Message {
        private String[] results;
        public FinishedMessage(String[] results) {
            this.results = results;
        }

        public void send(ClientSession session) throws IOException {
            StringBuilder sb = new StringBuilder();
            sb.append("message=FINISHED");
            for (int i = 0; i < results.length; ++i) {
                if (results[i] != null) {
                    sb.append("&lane").append(i + 1).append("=").append(results[i]);
                }
            }
            session.sendTimerMessage(sb.toString());
        }
    }

    public static class MalfunctionMessage implements Message {
        private String errorMsg;
        public MalfunctionMessage(String errorMsg) {
            this.errorMsg = errorMsg;
        }

        public void send(ClientSession session) throws IOException {
            session.sendTimerMessage("message=MALFUNCTION&error=" + URLEncoder.encode(errorMsg, "UTF-8"));
        }
    }

    public static class HeartbeatMessage implements Message {
        public void send(ClientSession session) throws IOException {
            session.sendTimerMessage("message=HEARTBEAT");  // TODO
        }
    }

    public HttpTask(String base_url, String username, String password) throws IOException {
        session = new ClientSession(base_url, username, password);
    }

    public void send(Message message) throws IOException {
        message.send(session);
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
