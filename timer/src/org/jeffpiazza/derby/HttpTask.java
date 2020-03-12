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
  private RemoteStartCallback remoteStartCallback;
  // Print queued Messages when actually sent.
  private boolean traceQueued;
  // Print heartbeat Messages when actually sent.
  private boolean traceHeartbeat;
  private boolean traceResponses;

  public static final long heartbeatPace = 500;  // ms.

  // Callbacks all get invoked from the thread running HttpTask.  They're
  // expected to return reasonably quickly to their caller.
  // Called when it's time to send a heartbeat to the web server, which we'll
  // do only if the timer is healthy (connected).
  public interface TimerHealthCallback {
    boolean isTimerHealthy();
  }

  // Called when a PREPARE_HEAT message is received from the web server
  public interface HeatReadyCallback {
    void onHeatReady(int roundid, int heat, int lanemask);
  }

  // Called when an ABORT_HEAT message is received from the web server
  public interface AbortHeatCallback {
    void onAbortHeat();
  }

  public interface RemoteStartCallback {
    boolean hasRemoteStart();

    void remoteStart();
  }

  public interface LoginCallback {
    void onLoginSuccess();

    void onLoginFailed(String message);
  }

  // When a ClientSession and credentials are available, start() launches
  // the HttpTask in a new Thread.
  public static void start(final ClientSession session,
                           final Connector connector,
                           final LoginCallback callback) {
    (new Thread() {
      @Override
      public void run() {
        boolean login_ok = false;
        try {
          Element login_response = session.login();
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
          HttpTask task = new HttpTask(session);
          connector.setHttpTask(task);
          task.run();
        }
      }
    }).start();
  }

  public HttpTask(ClientSession session) {
    this.session = session;
    this.queue = new ArrayList<Message>();
    this.traceQueued = traceQueued;
    this.traceHeartbeat = traceHeartbeat;
    this.traceResponses = traceResponses;
    synchronized (queue) {
      queueMessage(new Message.Hello());
    }
  }

  public void sendIdentified(int nlanes, String timer, String identifier) {
    synchronized (queue) {
      queueMessage(new Message.Identified(nlanes, timer, identifier));
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

  protected synchronized void registerRemoteStart(RemoteStartCallback cb) {
    this.remoteStartCallback = cb;
  }

  protected synchronized RemoteStartCallback getRemoteStartCallback() {
    return this.remoteStartCallback;
  }

  protected boolean hasRemoteStart() {
    RemoteStartCallback cb = getRemoteStartCallback();
    return cb != null && cb.hasRemoteStart();
  }

  private static int parseIntOrZero(String attr) {
    if (attr.isEmpty()) {
      return 0;
    }
    try {
      return Integer.valueOf(attr);
    } catch (NumberFormatException nfe) {
      // regex should have ensured this won't happen
      LogWriter.stacktrace(nfe);
      System.err.println(
          "Unexpected number format exception reading heat-ready response");
      return 0;
    }
  }

  // HttpTask has a queue for events to send, registers callbacks
  // for HEAT-READY(with lane mask) and ABORT.  Continually checks
  // queue, sending queued events; otherwise sends a HEARTBEAT and
  // sleeps a known amount of time.
  public void run() {
    System.err.println("Running HttpTask");
    while (true) {
      Element response = null;
      boolean trace;
      boolean log = true;
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
          trace = this.traceQueued;
        } else {
          TimerHealthCallback timerHealth = getTimerHealthCallback();
          if (timerHealth != null && timerHealth.isTimerHealthy()) {
            // Send heartbeats only if we've actually identified the timer and
            // it's healthy.
            nextMessage = new Message.Heartbeat();
            log = trace = this.traceHeartbeat;
          } else {
            continue;
          }
        }

        while (response == null) {
          String params = null;
          try {
            params = nextMessage.asParameters();
            if (hasRemoteStart()) {
              params += "&remote-start=YES";
            }
            if (trace) {
              StdoutMessageTrace.httpMessage(params);
            }
            if (log) {
              LogWriter.httpMessage(params);
            }
            response = session.sendTimerMessage(params);
          } catch (Throwable t) {
            LogWriter.trace("Unable to send HTTP message " + params);
            LogWriter.stacktrace(t);
            System.err.println(
                "Unable to send HTTP message " + params + "; retrying");
            t.printStackTrace();
          }
        }
      }

      if (!ClientSession.wasSuccessful(response)) {
        continue;
      }

      if (traceResponses) {
        if (trace) {
          StdoutMessageTrace.httpResponse(nextMessage, response);
        }
        if (log) {
          LogWriter.httpResponse(response);
        }
      }

      NodeList heatReadyNodes = response.getElementsByTagName("heat-ready");
      if (heatReadyNodes.getLength() > 0) {
        HeatReadyCallback cb = getHeatReadyCallback();
        if (cb != null) {
          Element heatReady = (Element) heatReadyNodes.item(0);
          int lanemask = parseIntOrZero(heatReady.getAttribute("lane-mask"));
          int roundid = parseIntOrZero(heatReady.getAttribute("roundid"));
          int heat = parseIntOrZero(heatReady.getAttribute("heat"));
          cb.onHeatReady(roundid, heat, lanemask);
        }
      }

      if (response.getElementsByTagName("abort").getLength() > 0) {
        AbortHeatCallback cb = getAbortHeatCallback();
        if (cb != null) {
          cb.onAbortHeat();
        }
      }

      if (response.getElementsByTagName("remote-start").getLength() > 0) {
        RemoteStartCallback cb = getRemoteStartCallback();
        if (cb != null) {
          cb.remoteStart();
        }
      }
    }
  }
}
