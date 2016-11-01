package org.jeffpiazza.derby;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.*;
import java.net.UnknownHostException;
import java.util.ArrayList;

// HttpTask expects to run in its own thread sending outgoing messages and
// awaiting their responses.  Sends a heartbeat message periodically if the
// queue remains empty.
public class HttpTask implements Runnable {
  private ClientSession session;
  // Messages waiting to be sent to web server
  private final ArrayList<Message> queue;
  private TimerHealthCallback timerHealthCallback;
  private HeatReadyCallback heatReadyCallback;
  private AbortHeatCallback abortHeatCallback;
  // Print queued Messages when actually sent.
  private MessageTracer traceQueued;
  // Print heartbeat Messages when actually sent.
  private MessageTracer traceHeartbeat;

  public static final long heartbeatPace = 10000;  // ms.

  // Callbacks all get invoked from the thread running HttpTask.  They're
  // expected to return reasonably quickly to their caller.

  // Called when it's time to send a heartbeat to the web server, which we'll
  // do only if the timer is healthy (connected).
  public interface TimerHealthCallback {
    boolean isTimerHealthy();
  }

  // Called when a PREPARE_HEAT message is received from the web server
  public interface HeatReadyCallback {
    void heatReady(int lanemask);
  }

  // Called when an ABORT_HEAT message is received from the web server
  public interface AbortHeatCallback {
    void abortHeat();
  }

  public interface MessageTracer {
    void onMessageSend(Message m, String params);

    void onMessageResponse(Message m, Element response);

    void traceInternal(String s);
  }

  public interface LoginCallback {
    void onLoginSuccess();

    void onLoginFailed(String message);
  }

  // When a ClientSession and credentials are available, start() launches
  // the HttpTask in a new Thread.
  public static void start(final String username, final String password,
                           final ClientSession session,
                           final MessageTracer traceQueued,
                           final MessageTracer traceHeartbeat,
                           final Connector connector,
                           final LoginCallback callback) {
    (new Thread() {
      @Override
      public void run() {
        boolean login_ok = false;
        try {
          Element login_response = session.login(username, password);
          login_ok = ClientSession.wasSuccessful(login_response);
          if (!login_ok) {
            callback.onLoginFailed("Login failed");
          }
        } catch (UnknownHostException e) {
          callback.onLoginFailed("Unknown host");
        } catch (IOException e) {
          callback.onLoginFailed(e.getMessage());
        }

        if (login_ok) {
          callback.onLoginSuccess();
          HttpTask task = new HttpTask(session, traceQueued, traceHeartbeat);
          connector.setHttpTask(task);
          task.run();
        }
      }
    }).start();
  }

  public HttpTask(ClientSession session, MessageTracer traceQueued,
                  MessageTracer traceHeartbeat) {
    this.session = session;
    this.queue = new ArrayList<Message>();
    this.traceQueued = traceQueued;
    this.traceHeartbeat = traceHeartbeat;
    synchronized (queue) {
      queueMessage(new Message.Hello());
    }
  }

  public void sendIdentified(int nlanes) {
    synchronized (queue) {
      queueMessage(new Message.Identified(nlanes));
    }
  }

  // Queues a Message to be sent to the web server.  This is the main entry
  // point once HttpTask is up and running.
  public void queueMessage(Message message) {
    synchronized (queue) {
      queue.add(message);
      queue.notifyAll();
    }
  }

  public synchronized void registerTimerHealthCallback(TimerHealthCallback cb) {
    this.timerHealthCallback = cb;
  }

  protected synchronized TimerHealthCallback getTimerHealthCallback() {
    return this.timerHealthCallback;
  }
  public synchronized void registerHeatReadyCallback(HeatReadyCallback cb) {
    this.heatReadyCallback = cb;
  }

  protected synchronized HeatReadyCallback getHeatReadyCallback() {
    return this.heatReadyCallback;
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
          TimerHealthCallback timerHealth = getTimerHealthCallback();
          if (timerHealth != null && timerHealth.isTimerHealthy()) {
            // Send heartbeats only if we've actually identified the timer and
            // it's healthy.
            nextMessage = new Message.Heartbeat();
            traceMessage = this.traceHeartbeat;
          } else {
            continue;
          }
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

      if (!ClientSession.wasSuccessful(response)) {
        continue;
      }

      if (traceMessage != null) {
        traceMessage.onMessageResponse(nextMessage, response);
      }

      NodeList heatReady = response.getElementsByTagName("heat-ready");
      if (heatReady.getLength() > 0) {
        try {
          int lanemask = Integer.valueOf(((Element) heatReady.item(0)).
              getAttribute("lane-mask"));
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
