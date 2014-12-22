package org.jeffpiazza.derby;

public class CombinedMessageTracer implements HttpTask.MessageTracer {
  private HttpTask.MessageTracer[] tracers;

  public CombinedMessageTracer(HttpTask.MessageTracer t1,
                               HttpTask.MessageTracer t2) {
    this.tracers = new HttpTask.MessageTracer[] { t1, t2 };
  }

  public void onMessageSend(Message m, String params) {
    for (HttpTask.MessageTracer t : tracers) {
      t.onMessageSend(m, params);
    }
  }

  public void onMessageResponse(Message m, String response) {
    for (HttpTask.MessageTracer t : tracers) {
      t.onMessageResponse(m, response);
    }
  }
}
