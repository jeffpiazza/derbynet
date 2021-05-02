package org.jeffpiazza.derby.timer;

// Note that there's no LOST_CONNECTION, because if that happens, we're
import java.util.ArrayDeque;
import java.util.ArrayList;
import org.jeffpiazza.derby.LogWriter;

// going to abandon the state machine and everything else altogether.
public enum Event {
  PREPARE_HEAT_RECEIVED,
  ABORT_HEAT_RECEIVED, // GATE_OPEN and GATE_CLOSED may be repeatedly signaled
  GATE_OPEN,
  GATE_CLOSED,
  RACE_STARTED,
  RACE_FINISHED, // An OVERDUE event signals that expected results have not yet arrived
  LANE_RESULT, // Just one
  OVERDUE, // Eventually, overdue results give way to a GIVING_UP event,
  // which is roughly treated like another PREPARE_HEAT_RECEIVED.
  GIVING_UP,
  LANE_COUNT // Some timers report how many lanes
  ;

  public static interface Handler {
    void onEvent(Event event, String[] args);
  }

  private static ArrayList<Handler> handlers = new ArrayList<Handler>();

  public static void register(Handler handler) {
    handlers.add(handler);
  }

  public static void unregister(Handler handler) {
    handlers.remove(handler);
  }

  public static void trigger(Event event) {
    trigger(event, null);
  }

  public static void trigger(Event event, String[] args) {
    triggerOne(event, args);
    drainQueue();
  }

  protected static void triggerOne(Event event, String[] args) {
    for (Handler handler : handlers) {
      handler.onEvent(event, args);
    }
  }

  // If needed, the queue could support an Event enum plus args
  private static ArrayDeque<Event> queue = new ArrayDeque<Event>();

  public static void queue(Event event) {
    synchronized (queue) {
      queue.add(event);
    }
  }

  private static void drainQueue() {
    while (true) {
      Event e;
      synchronized (queue) {
        e = queue.poll();
      }
      if (e == null) {
        return;
      }
      triggerOne(e, null);
    }
  }
}
