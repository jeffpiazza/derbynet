package org.jeffpiazza.derby;

import org.jeffpiazza.derby.devices.TimerTask;

// A Connector is what joins the HttpTask (talking to the web server) with
// the TimerDevice (talking to the physical timer).  When both are available, the connector wires them together
// (establishes callbacks between the two) and queues a Hello message to the web server.
public interface Connector {
  void setHttpTask(HttpTask httpTask);
  void setTimerTask(TimerTask deviceTask);
}
