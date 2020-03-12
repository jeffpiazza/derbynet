package org.jeffpiazza.derby;

import java.util.ArrayList;
import java.util.List;
import org.w3c.dom.Element;

public abstract class Flag<T> {
  private static List<Flag> all_flags = new ArrayList<Flag>();

  public static final Flag<Boolean> version
      = new BooleanFlag("v", "Show version");

  public static final Flag<Boolean> headless
      = new BooleanFlag("x", "Run headless, without GUI.");

  public static final Flag<String> logdir
      = new StringFlag("logdir", null, "Write log files in <directory>");

  public static final Flag<Boolean> trace_messages
      = new BooleanFlag("t", "Trace non-heartbeat messages sent");
  public static final Flag<Boolean> trace_heartbeats
      = new BooleanFlag("th", "Trace heartbeat messages sent");
  public static final Flag<Boolean> trace_responses
      = new BooleanFlag("r", "Trace responses to traced messages");

  public static final Flag<String> username
      = new StringFlag("u", "Timer",
                       "Specify username for authenticating to web server");
  public static final Flag<String> password
      = new StringFlag("p", "",
                       "Specify password for authenticating to web server");
  public static final Flag<String> portname
      = new StringFlag("n", null,
                       "Use specified port name instead of searching");
  public static final Flag<String> devicename
      = new StringFlag("d", null,
                       "Use specified device instead of trying to identify");

  public static final Flag<Boolean> ignore_place
      = new BooleanFlag("ignore-place",
                        "Discard any place indications from timer");

  public static final Flag<Long> delay_reset_after_race
      = new LongFlag("delay-reset-after-race", 10,
                     "How long after race over before timer will be reset,"
                     + " default 10s.  (For SmartLine, DerbyMagic, NewBold,"
                     + " and BertDrake.)");
  // Issue #35: Reject gate state changes that don't last "reasonably" long.
  // To do that, don't record a gate state change until it's aged a bit.
  //
  public static final Flag<Long> min_gate_time
      = new LongFlag("min-gate-time", 0,
                     "Ignore gate transitions shorter than <milliseconds>");

  public static final Flag<Boolean> simulate_timer
      = new BooleanFlag("simulate-timer", "Simulate timer device (for testing)");
  public static final Flag<Boolean> simulate_host
      = new BooleanFlag("simulate-host", "Exercise timer with simulated host");
  public static final Flag<Integer> lanes
      = new IntegerFlag("lanes", 0, "Specify number of lanes to report");
  public static final Flag<Integer> pace
      = new IntegerFlag("pace", 0, "Staging pace (seconds between heats)");

  public static final Flag<Boolean> record
      = new BooleanFlag("record", null);
  public static final Flag<String> playback
      = new StringFlag("playback", null, null);

  public static final Flag<Integer> reset_after_start
      = new IntegerFlag("reset-after-start", 10,
                        "TheJudge: Reset timer <nsec> seconds after heat "
                        + "start, default 10");

  public static final Flag<Boolean> skip_enhanced_format
      = new BooleanFlag("skip-enhanced-format",
                        "FastTrack: Don't attempt enhanced format command");
  public static final Flag<Boolean> skip_read_features
      = new BooleanFlag("skip-read-features",
                        "FastTrack: Don't attempt reading features");

  public Flag(String name, T value, String description) {
    this.name = name;
    this.value = value;
    this.description = description;
    all_flags.add(this);
  }

  public static int parseCommandLineFlags(String[] args, int argc) {
    boolean advanced;
    do {
      advanced = false;
      for (int i = 0; i < all_flags.size(); ++i) {
        int new_argc = all_flags.get(i).maybeParseCommandLine(args, argc);
        if (new_argc != argc) {
          argc = new_argc;
          advanced = true;
        }
      }
    } while (advanced);
    return argc;
  }

  public static void usage() {
    for (int i = 0; i < all_flags.size(); ++i) {
      System.err.println(all_flags.get(i).usage_string());
    }
  }

  public abstract int maybeParseCommandLine(String[] args, int argc);

  public abstract boolean maybeParseXml(Element flag_element);

  public String name() {
    return name;
  }

  public T value() {
    return value;
  }

  public String usage_string() {
    return "   -" + name() + ": " + description;
  }

  protected final String name;
  protected T value;
  private String description;

  public static class BooleanFlag extends Flag<Boolean> {
    public BooleanFlag(String name, String description) {
      super(name, Boolean.FALSE, description);
    }

    @Override
    public int maybeParseCommandLine(String[] args, int argc) {
      if (argc >= args.length) {
        return argc;
      }
      if (args[argc].equals("-" + name)) {
        value = Boolean.TRUE;
        return argc + 1;
      }
      if (args[argc].equals("-no-" + name)) {
        value = Boolean.FALSE;
        return argc + 1;
      }
      return argc;
    }

    @Override
    public boolean maybeParseXml(Element flag_element) {
      return false;  // TODO
    }
  }

  public static class StringFlag extends Flag<String> {
    public StringFlag(String name, String value, String description) {
      super(name, value, description);
    }

    @Override
    public int maybeParseCommandLine(String[] args, int argc) {
      if (argc + 1 >= args.length) {
        return argc;
      }
      if (args[argc].equals("-" + name)) {
        value = args[argc + 1];
        return argc + 2;
      }
      return argc;
    }

    @Override
    public boolean maybeParseXml(Element flag_element) {
      return false;  // TODO
    }
  }

  public static class LongFlag extends Flag<Long> {
    public LongFlag(String name, Long value, String description) {
      super(name, value, description);
    }

    public LongFlag(String name, long value, String description) {
      super(name, new Long(value), description);
    }

    @Override
    public int maybeParseCommandLine(String[] args, int argc) {
      if (argc + 1 >= args.length) {
        return argc;
      }
      if (args[argc].equals("-" + name)) {
        value = Long.parseLong(args[argc + 1]);
        return argc + 2;
      }
      return argc;
    }

    @Override
    public boolean maybeParseXml(Element flag_element) {
      return false;  // TODO
    }
  }

  public static class IntegerFlag extends Flag<Integer> {
    public IntegerFlag(String name, Integer value, String description) {
      super(name, value, description);
    }

    @Override
    public int maybeParseCommandLine(String[] args, int argc) {
      if (argc + 1 >= args.length) {
        return argc;
      }
      if (args[argc].equals("-" + name)) {
        value = Integer.parseInt(args[argc + 1]);
        return argc + 2;
      }
      return argc;
    }

    @Override
    public boolean maybeParseXml(Element flag_element) {
      return false;  // TODO
    }
  }
}
