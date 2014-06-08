package org.jeffpiazza.derby;

import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

public class HttpTask implements Runnable {
    private ClientSession session;
    private ArrayList<Message> queue;  // Messages waiting to be sent to web server
    private HeatReadyCallback heatReadyCallback;
    private AbortHeatCallback abortHeatCallback;
    private boolean traceQueued;  // Print queued Messages when actually sent.
    private boolean traceHeartbeat;  // Print heartbeat Messages when actually sent.
    private boolean traceResponses;  // Print responses to traced messages

    public HttpTask(String base_url, String username, String password,
                    boolean traceQueued, boolean traceHeartbeat, boolean traceResponses) throws IOException {
        this.session = new ClientSession(base_url, username, password);
        this.queue = new ArrayList<Message>();
        this.traceQueued = traceQueued;
        this.traceHeartbeat = traceHeartbeat;
        this.traceResponses = traceResponses;
    }

    public HttpTask(String base_url, String username, String password) throws IOException {
        this(base_url, username, password, false, false, false);
    }

    public void send(Message message) {
        synchronized (queue) {
            queue.add(message);
            queue.notifyAll();
        }
    }

    public interface HeatReadyCallback {
        void heatReady(int lanemask);
    }
    public synchronized void registerHeatReadyCallback(HeatReadyCallback cb) {
        this.heatReadyCallback = cb;
    }
    protected synchronized HeatReadyCallback getHeatReadyCallback() { return this.heatReadyCallback; }

    public interface AbortHeatCallback {
        void abortHeat();
    }
    public synchronized void registerAbortHeatCallback(AbortHeatCallback cb) {
        this.abortHeatCallback = cb;
    }
    protected synchronized AbortHeatCallback getAbortHeatCallback() { return this.abortHeatCallback; }

    private static final Pattern heatReadyPattern = Pattern.compile("<heat-ready lane-mask=\"([0-9]+)\"");

    // HttpTask has a queue for events to send, registers callbacks
    // for HEAT-READY(with lane mask) and ABORT.  Continually checks
    // queue, sending queued events; otherwise sends a HEARTBEAT and
    // sleeps a known amount of time.
    public void run() {
        while (true) {
            String response = "";
            boolean traceMessage = false;
            synchronized (queue) {
                if (queue.size() == 0) {
                    try {
                        queue.wait(30000);  // ms.
                    } catch (InterruptedException e) {}
                }
                Message nextMessage;
                if (queue.size() > 0) {
                    nextMessage = queue.remove(0);
                    traceMessage = this.traceQueued;
                } else {
                    nextMessage = new Message.Heartbeat();
                    traceMessage = this.traceHeartbeat;
                }

                boolean succeeded = false;
                while (!succeeded) {
                    try {
                        String params = nextMessage.asParameters();
                        if (traceMessage) {
                            System.out.println("    sending " + params);
                        }
                        response = session.sendTimerMessage(params);
                        succeeded = true;
                    } catch (Throwable t) {
                        System.out.println("Unable to send timer message; retrying");
                    }
                }
            }

            // Cheesy string matching suffices for now, but should be
            // made more XML-aware if we're going to send richer XML.
            if (response.indexOf("<success") < 0 || response.indexOf("<failure") >= 0) {
                System.out.println("Message resulted in failure");
                System.out.println("=======================");
                System.out.println(response);
                System.out.println("=======================");
            }
            else if (traceMessage && this.traceResponses) {
                System.out.println("    response: {{ " + response + "}}");
            }

            Matcher hrMatcher = heatReadyPattern.matcher(response);
            if (hrMatcher.find()) {
                try {
                    int lanemask = Integer.valueOf(hrMatcher.group(1));
                    HeatReadyCallback cb = getHeatReadyCallback();
                    if (cb != null) {
                        cb.heatReady(lanemask);
                    }
                } catch (NumberFormatException nfe) { // regex should have ensured this won't happen
                    System.out.println("Unexpected number format exception reading heat-ready response");
                }
            }
            if (response.indexOf("<abort/>") >= 0) {
                AbortHeatCallback cb = getAbortHeatCallback();
                if (cb != null) {
                    cb.abortHeat();
                }
            }
        }
    }
}
