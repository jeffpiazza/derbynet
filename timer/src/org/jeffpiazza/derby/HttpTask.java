package org.jeffpiazza.derby;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.*;
import java.net.UnknownHostException;
import java.util.ArrayList;
import org.json.JSONObject;

// HttpTask expects to run in its own thread sending outgoing messages and
// awaiting their responses.  Sends a heartbeat message periodically if the
// queue remains empty.
public class HttpTask implements Runnable {
  private ClientSession session;
  private boolean shouldExit = false;
  // Messages waiting to be sent to web server
  private final ArrayList<Message> queue;
  private TimerHealthCallback timerHealthCallback;
  private HeatReadyCallback heatReadyCallback;
  private AbortHeatCallback abortHeatCallback;
  private RemoteStartCallback remoteStartCallback;
  private AssignPortCallback assignPortCallback;
  private AssignDeviceCallback assignDeviceCallback;
  // Store credentials for automatic re-authentication
  private String role;
  private String password;

  public static final long heartbeatPace = 500;  // ms.

  // Callbacks all get invoked from the thread running HttpTask.  They're
  // expected to return reasonably quickly to their caller.
  // Called when it's time to send a heartbeat to the web server, which we'll
  // do only if the timer is healthy (connected).
  public interface TimerHealthCallback {
    public static final int UNHEALTHY = 0;
    public static final int PRESUMED_HEALTHY = 1;
    public static final int HEALTHY = 2;

    int getTimerHealth();
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

  public interface AssignPortCallback {
    void onAssignPort(String portName);
  }

  public interface AssignDeviceCallback {
    void onAssignDevice(String deviceName);
  }

  // When a ClientSession and credentials are available, start() launches
  // the HttpTask in a new Thread.
  public static void start(final ClientSession session,
                           final Connector connector,
                           final String role, final String password,
                           final LoginCallback callback) {
    // TODO This gets called by RoleFinder and by the timer GUI directly,
    // resulting in two HELLO messages to the server.  Need to figure out why.
    LogWriter.setClientSession(session);
    (new Thread() {
      @Override
      public void run() {
        boolean login_ok = false;
        try {
          JSONObject login_response = session.login(role, password);
          login_ok = ClientSession.wasSuccessful(login_response);
          if (!login_ok) {
            callback.onLoginFailed("Login failed");
          }
        } catch (UnknownHostException e) {
          callback.onLoginFailed("Unknown host");
        } catch (IOException e) {  // Including ClientSession.HttpException
          callback.onLoginFailed(e.getMessage());
        }

        if (login_ok) {
          callback.onLoginSuccess();
          HttpTask task = new HttpTask(session, role, password);
          connector.setHttpTask(task);
          task.run();
        }
      }
    }).start();
  }

  public HttpTask(ClientSession session, String role, String password) {
    this.session = session;
    this.role = role;
    this.password = password;
    this.queue = new ArrayList<Message>();
    synchronized (queue) {
      queueMessage(new Message.Hello());
    }
    LogWriter.info("Connection established to " + session.getBaseUrl());
  }

  public void sendIdentified(int nlanes, String timer, String humanName,
                             String identifier, boolean confirmed) {
    synchronized (queue) {
      queueMessage(new Message.Identified(nlanes, timer, humanName, identifier,
                                          confirmed));
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

  public synchronized void setShouldExit() { shouldExit = true; }
  private synchronized boolean getShouldExit() { return shouldExit; }

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

  protected synchronized void registerAssignPortCallback(AssignPortCallback cb) {
    this.assignPortCallback = cb;
  }

  protected synchronized AssignPortCallback getAssignPortCallback() {
    return assignPortCallback;
  }

  protected synchronized void registerAssignDeviceCallback(
      AssignDeviceCallback cb) {
    this.assignDeviceCallback = cb;
  }

  protected synchronized AssignDeviceCallback getAssignDeviceCallback() {
    return assignDeviceCallback;
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

  // Check if the response indicates "not authorized"
  private boolean isNotAuthorized(Element response) {
    if (response == null) {
      return false;
    }
    NodeList failures = response.getElementsByTagName("failure");
    for (int i = 0; i < failures.getLength(); i++) {
      Element failure = (Element) failures.item(i);
      String code = failure.getAttribute("code");
      if ("notauthorized".equals(code)) {
        return true;
      }
    }
    return false;
  }

  // Attempt to re-authenticate with the server
  private boolean reAuthenticate() {
    try {
      LogWriter.info("Session lost, attempting to re-authenticate...");
      System.err.println(Timestamp.string() + ": Session lost, re-authenticating...");
      JSONObject login_response = session.login(role, password);
      boolean success = ClientSession.wasSuccessful(login_response);
      if (success) {
        LogWriter.info("Re-authentication successful");
        System.err.println(Timestamp.string() + ": Re-authentication successful");
      } else {
        LogWriter.info("Re-authentication failed");
        System.err.println(Timestamp.string() + ": Re-authentication failed");
      }
      return success;
    } catch (IOException e) {
      LogWriter.stacktrace(e);
      System.err.println(Timestamp.string() + ": Re-authentication error: " + e.getMessage());
      return false;
    }
  }

  // HttpTask has a queue for events to send, registers callbacks
  // for HEAT-READY(with lane mask) and ABORT.  Continually checks
  // queue, sending queued events; otherwise sends a HEARTBEAT and
  // sleeps a known amount of time.
  public void run() {
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
          trace = Flag.trace_messages.value();
        } else if (getShouldExit()) {
          return;
        } else {
          TimerHealthCallback timerHealth = getTimerHealthCallback();
          if (timerHealth != null) {
            nextMessage = new Message.Heartbeat(timerHealth.getTimerHealth());
            log = trace = Flag.trace_heartbeats.value();
          } else {
            // This really shouldn't arise: we always register a timer health callback
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
              if (nextMessage.getClass() == Message.Flags.class) {
                LogWriter.httpMessage("message=FLAGS&...");
              } else {
                LogWriter.httpMessage(params);
              }
            }
            response = session.sendTimerMessage(params);
          } catch (ClientSession.HttpException he) {
            LogWriter.httpResponse(he.getMessage());
            // Brief delay before retry to avoid hammering a recovering server
            try {
              Thread.sleep(1000);  // 1 second
            } catch (InterruptedException ie) {
            }
          } catch (Throwable t) {
            LogWriter.trace("Unable to send HTTP message " + params);
            LogWriter.stacktrace(t);
            // Brief delay before retry to avoid hammering a recovering server
            try {
              Thread.sleep(1000);  // 1 second
            } catch (InterruptedException ie) {
            }
          }
        }
      }

      // Check for "not authorized" and attempt re-authentication
      if (isNotAuthorized(response)) {
        if (reAuthenticate()) {
          // Re-authentication successful, retry the message
          synchronized (queue) {
            queue.add(0, nextMessage);  // Put message back at front of queue
          }
          continue;  // Skip to next iteration to retry the message
        }
        // Re-authentication failed, continue to log the failure below
      }

      if (ClientSession.wasSuccessful(response)) {
        // An unsuccessful response will always be logged; for successful
        // responses, we log only based on flags.
        if (Flag.trace_responses.value()) {
          if (trace) {
            StdoutMessageTrace.httpResponse(nextMessage, response);
          }
          if (log) {
            LogWriter.httpResponse(response);
          }
        }
      }

      decodeResponse(response);
    }
  }

  private void decodeResponse(Element response) {
    NodeList nodes = response.getElementsByTagName("remote-log");
    if (nodes.getLength() > 0) {
      LogWriter.setRemoteLogging(Boolean.parseBoolean(
          ((Element) nodes.item(0)).getAttribute("send")));
    }

    if (response.getElementsByTagName("abort").getLength() > 0) {
      LogWriter.httpResponse("<abort>");
      AbortHeatCallback cb = getAbortHeatCallback();
      if (cb != null) {
        cb.onAbortHeat();
      }
    }

    if ((nodes = response.getElementsByTagName("heat-ready")).getLength() > 0) {
      Element heatReady = (Element) nodes.item(0);
      int lanemask = parseIntOrZero(heatReady.getAttribute("lane-mask"));
      int roundid = parseIntOrZero(heatReady.getAttribute("roundid"));
      int heat = parseIntOrZero(heatReady.getAttribute("heat"));
      LogWriter.httpResponse(
          "<heat-ready: roundid=" + roundid + ", heat=" + heat + ">");
      HeatReadyCallback cb = getHeatReadyCallback();
      if (cb != null) {
        cb.onHeatReady(roundid, heat, lanemask);
      }
    }

    if (response.getElementsByTagName("remote-start").getLength() > 0) {
      LogWriter.httpResponse("<remote-start>");
      RemoteStartCallback cb = getRemoteStartCallback();
      if (cb != null) {
        cb.remoteStart();
      }
    }

    nodes = response.getElementsByTagName("assign-flag");
    for (int i = 0; i < nodes.getLength(); ++i) {
      Element assignment = (Element) nodes.item(0);
      String flagName = assignment.getAttribute("flag");
      String value = assignment.getAttribute("value");
      LogWriter.httpResponse("<assign-flag " + flagName + ": " + value + ">");
      Flag.assignFlag(flagName, value);
    }

    if ((nodes = response.getElementsByTagName("assign-port")).getLength() > 0) {
      String portName = ((Element) nodes.item(0)).getAttribute("port");
      LogWriter.httpResponse("<assign-port " + portName + ">");
      AssignPortCallback cb = getAssignPortCallback();
      if (cb != null) {
        cb.onAssignPort(portName);
      }
    }

    if ((nodes = response.getElementsByTagName("assign-device")).getLength() > 0) {
      String deviceName = ((Element) nodes.item(0)).getAttribute("device");
      LogWriter.httpResponse("<assign-device " + deviceName + ">");
      AssignDeviceCallback cb = getAssignDeviceCallback();
      if (cb != null) {
        cb.onAssignDevice(deviceName);
      }
    }

    if (response.getElementsByTagName("query").getLength() > 0) {
      LogWriter.httpResponse("<query>");
      queueMessage(new Message.Flags());
    }
  }
}
