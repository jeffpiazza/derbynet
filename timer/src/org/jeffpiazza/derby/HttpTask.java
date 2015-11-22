package org.jeffpiazza.derby;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.*;
import java.net.UnknownHostException;
import java.util.ArrayList;

// HttpTask expects to run in its own thread sending outgoing messages and awaiting their responses.  Sends a
// heartbeat message periodically if the queue remains empty.
//
// TODO If heartbeat stops, present message to user.
public class HttpTask implements Runnable {
  private ClientSession session;
  private boolean awaitingHello = false;
  private final ArrayList<Message> queue;  // Messages waiting to be sent to web server
  private HeatReadyCallback heatReadyCallback;
  private AbortHeatCallback abortHeatCallback;
  private MessageTracer traceQueued;  // Print queued Messages when actually sent.
  private MessageTracer traceHeartbeat;  // Print heartbeat Messages when actually sent.

  public static final long heartbeatPace = 10000;  // ms.

  // HeatReadyCallback, AbortHeatCallback, and MessageTracer methods all get invoked from the thread running HttpTask.
  // They're expected to return reasonably quickly to their caller.
  public interface HeatReadyCallback {
    void heatReady(int lanemask);
  }

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

  public static void start(final String username, final String password, final ClientSession session,
                           final MessageTracer traceQueued, final MessageTracer traceHeartbeat, final Connector connector,
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

  public HttpTask(ClientSession session, MessageTracer traceQueued, MessageTracer traceHeartbeat) {
    this.session = session;
    this.queue = new ArrayList<Message>();
    this.traceQueued = traceQueued;
    this.traceHeartbeat = traceHeartbeat;
  }

  public void sendHello(int nlanes) {
    System.out.println("Queuing HELLO message");  // TODO - Test this behaves correctly in command-line mode
    synchronized (queue) {
      awaitingHello = false;
      queueMessage(new Message.Hello(nlanes));
    }
  }

  // Queues a Message to be sent to the web server.  This is the main entry point once HttpTask is up and running.
  public void queueMessage(Message message) {
    synchronized (queue) {
      if (!awaitingHello) {
        queue.add(message);
        queue.notifyAll();
      }
    }
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

  // TODO No heartbeats until timer device has been discovered and hello message has been sent?

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
        } else if (awaitingHello) {
          // Don't send heartbeats while we're still waiting for the first hello message; go back and sleep some more
          continue;
        } else {
          // Send heartbeats only if we've done at least one hello
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

      if (!ClientSession.wasSuccessful(response)) {
        continue;
      }

      if (traceMessage != null) {
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
