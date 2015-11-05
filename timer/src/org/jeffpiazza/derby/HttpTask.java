package org.jeffpiazza.derby;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

public class HttpTask implements Runnable {
    private ClientSession session;
    private ArrayList<Message> queue;  // Messages waiting to be sent to web server
    private HeatReadyCallback heatReadyCallback;
    private AbortHeatCallback abortHeatCallback;
    private MessageTracer traceQueued;  // Print queued Messages when actually sent.
    private MessageTracer traceHeartbeat;  // Print heartbeat Messages when actually sent.

    public static final long heartbeatPace = 10000;  // ms.

    public interface MessageTracer {
        void onMessageSend(Message m, String params);

        void onMessageResponse(Message m, Element response);

        void traceInternal(String s);
    }

    public HttpTask(String base_url, String username, String password,
                    MessageTracer traceQueued, MessageTracer traceHeartbeat) throws IOException {
        this.session = new ClientSession(base_url, username, password);
        this.queue = new ArrayList<Message>();
        this.traceQueued = traceQueued;
        this.traceHeartbeat = traceHeartbeat;
    }

    public HttpTask(String base_url, String username, String password) throws IOException {
        this(base_url, username, password, null, null);
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

    protected synchronized HeatReadyCallback getHeatReadyCallback() {
        return this.heatReadyCallback;
    }

    public interface AbortHeatCallback {
        void abortHeat();
    }

    public synchronized void registerAbortHeatCallback(AbortHeatCallback cb) {
        this.abortHeatCallback = cb;
    }

    protected synchronized AbortHeatCallback getAbortHeatCallback() {
        return this.abortHeatCallback;
    }

    // HttpTask has a queue for events to send, registers callbacks
    // for HEAT-READY(with lane mask) and ABORT.  Continually checks
    // queue, sending queued events; otherwise sends a HEARTBEAT and
    // sleeps a known amount of time.
    public void run() {
        while (true) {
            Element response = null;
            MessageTracer traceMessage = null;
            Message nextMessage;
            synchronized (queue) {
                if (queue.size() == 0) {
                    try {
                        queue.wait(heartbeatPace);  // ms.
                    } catch (InterruptedException e) {
                    }
                }
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
                        if (traceMessage != null) {
                            traceMessage.onMessageSend(nextMessage, params);
                        }
                        response = session.sendTimerMessage(params);
                        succeeded = true;
                    } catch (Throwable t) {
                        System.out.println(Timestamp.string()
                                + ": Unable to send timer message; retrying");
                    }
                }
            }

            if (response == null) {
                System.out.println(Timestamp.string() + ": Unparseable response for message");
            } else if (response.getElementsByTagName("success").getLength() == 0
                    || response.getElementsByTagName("failure").getLength() > 0) {
                System.out.println(Timestamp.string() + ": Message resulted in failure");
                System.out.println("=======================");
                System.out.println(XmlSerializer.serialized(response));
                System.out.println("=======================");
            } else if (traceMessage != null) {
                traceMessage.onMessageResponse(nextMessage, response);
            }


            NodeList heatReady = response.getElementsByTagName("heat-ready");
            if (heatReady.getLength() > 0) {
                try {
                    int lanemask = Integer.valueOf(((Element) heatReady.item(0)).getAttribute("lane-mask"));
                    HeatReadyCallback cb = getHeatReadyCallback();
                    if (cb != null) {
                        cb.heatReady(lanemask);
                    }
                } catch (NumberFormatException nfe) { // regex should have ensured this won't happen
                    System.out.println(Timestamp.string()
                            + ": Unexpected number format exception reading heat-ready response");
                }
            }
            if (response.getElementsByTagName("abort").getLength() > 0) {
                AbortHeatCallback cb = getAbortHeatCallback();
                if (cb != null) {
                    cb.abortHeat();
                }
            }
        }
    }
}
