package org.jeffpiazza.derby.timer;

import java.util.Comparator;
import java.util.TreeSet;

public class ScheduledEventsQueue {
  private static class ScheduledEvent {
    private long deadline;
    private Event event;

    public ScheduledEvent(long deadline, Event event) {
      this.deadline = deadline;
      this.event = event;
    }
  }

  private TreeSet<ScheduledEvent> events
      = new TreeSet<ScheduledEvent>(new Comparator<ScheduledEvent>() {
        public int compare(ScheduledEvent e1, ScheduledEvent e2) {
          return Long.compare(e1.deadline, e2.deadline);
        }
      });

  public void addAt(long deadline, Event event) {
    events.add(new ScheduledEvent(deadline, event));
  }

  public void addAfterMs(long delay, Event event) {
    addAt(System.currentTimeMillis() + delay, event);
  }

  public void poll() {
    while (!events.isEmpty()) {
      ScheduledEvent ev = events.first();
      if (ev.deadline > System.currentTimeMillis()) {
        return;
      }

      events.remove(ev);
      Event.trigger(ev.event);
    }
  }
}
