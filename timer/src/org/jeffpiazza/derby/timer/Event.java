package org.jeffpiazza.derby.timer;

// TODO Thread safety for handler register/unregister.
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.TreeSet;
import org.jeffpiazza.derby.LogWriter;

// Note that there's no LOST_CONNECTION, because if that happens, we're
// going to abandon the state machine and everything else altogether.
public enum Event {
  PREPARE_HEAT_RECEIVED,
  ABORT_HEAT_RECEIVED,
  // GATE_OPEN and GATE_CLOSED may be repeatedly signaled
  GATE_OPEN,
  GATE_CLOSED,
  // PREPARE_HEAT_RECEIVED marks the transition from IDLE to (on your) MARK.
  // GET_SET marks the transition from MARK to SET
  GET_SET,

  RACE_STARTED,
  // RACE_FINISHED gets signaled either when all the expected lane results have
  // been reported, or (for some timers) when the timer says it's done.
  // Processing for RACE_FINISHED must be idempotent, as there may be two or
  // more RACE_FINISHED events triggered for the same race.
  RACE_FINISHED,
  LANE_RESULT, // Just one
  NO_MORE_RESULTS, // Fill in any results that were not populated from the timer (Only for timers that skip reporting)
  PARTIAL_LANE_RESULT_LANE_NUM, // First of 2 events to make a lane result
  PARTIAL_LANE_RESULT_TIME, // Last of 2 events to make a lane result, combines first and fires a LANE_RESULT
  // An OVERDUE event signals that expected results have not yet arrived
  OVERDUE,
  // Eventually, overdue results give way to a GIVING_UP event,
  // which is roughly treated like another PREPARE_HEAT_RECEIVED.
  GIVING_UP,
  LANE_COUNT, // Some timers report how many lanes
  GATE_WATCHER_NOT_SUPPORTED,
  // Some flag changes can affect timer profiles; existing TimerDeviceWithProfile
  // instances should reload the profile.
  PROFILE_UPDATED,
  // Some FastTrack timers may not support "LR" laser reset command, which we
  // try to send during MARK
  FASTTRACK_NO_LASER_RESET;

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

  public static void send(Event event) {
    send(event, null);
  }

  public static void send(Event event, String[] args) {
    synchronized (eventsQueue) {
      eventsQueue.add(new EventRecord(event, args));
      eventsQueue.notifyAll();
    }
  }

  public static void sendAt(long deadline, Event event) {
    synchronized (eventsQueue) {
      delayedEvents.
          add(new DelayedEvent(deadline, new EventRecord(event, null)));
      eventsQueue.notifyAll();
    }
  }

  public static void sendAfterMs(long delay, Event event) {
    sendAt(System.currentTimeMillis() + delay, event);
  }

  private static class EventRecord {
    private Event event;
    private String[] args;

    public EventRecord(Event event, String[] args) {
      this.event = event;
      this.args = args;
    }

    public Event event() {
      return event;
    }

    public String[] args() {
      return args;
    }
  }
  private static ArrayDeque<EventRecord> eventsQueue
      = new ArrayDeque<EventRecord>();

  private static class DelayedEvent {
    private long deadline;
    private EventRecord event;

    public DelayedEvent(long deadline, EventRecord event) {
      this.deadline = deadline;
      this.event = event;
    }
  }
  private static TreeSet<DelayedEvent> delayedEvents
      = new TreeSet<DelayedEvent>(new Comparator<DelayedEvent>() {
        public int compare(DelayedEvent e1, DelayedEvent e2) {
          return Long.compare(e1.deadline, e2.deadline);
        }
      });

  private static Thread eventPoller = new Thread() {
    @Override
    public void run() {
      while (true) {
        EventRecord e = null;
        synchronized (eventsQueue) {
          long timeout = 10000;  // Arbitrarily high
          if (!delayedEvents.isEmpty()) {
            // Wake up for the next delayed event's deadline
            timeout = delayedEvents.first().deadline
                - System.currentTimeMillis();
          }
          if (eventsQueue.isEmpty() && timeout > 0) {
            try {
              eventsQueue.wait(timeout);
            } catch (InterruptedException ex) {
            }
          }
          if (!eventsQueue.isEmpty()) {
            e = eventsQueue.poll();
          }
          if (!delayedEvents.isEmpty()) {
            DelayedEvent firstDelayed = delayedEvents.first();
            if (e == null && firstDelayed.deadline <= System.currentTimeMillis()) {
              delayedEvents.remove(firstDelayed);
              e = firstDelayed.event;
            }
          }
        }
        if (e != null) {
          for (Handler handler : Event.handlers) {
            try {
              handler.onEvent(e.event(), e.args());
            } catch (Throwable th) {
              LogWriter.info("Caught throwable from handler for "
                  + e.event().name());
              LogWriter.stacktrace(th);
            }
          }
        }
      }
    }
  };

  static {
    eventPoller.start();
  }

}
