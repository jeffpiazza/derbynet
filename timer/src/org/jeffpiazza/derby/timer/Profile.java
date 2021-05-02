package org.jeffpiazza.derby.timer;

import java.util.ArrayList;
import java.util.Map;
import java.util.TreeMap;

public class Profile {
  public String name;

  public Profile(String name) {
    this.name = name;
  }

  public static Profile forTimer(String name) {
    return new Profile(name);
  }

  public String end_of_line = "";

  public Profile end_of_line(String eol) {
    this.end_of_line = eol;
    return this;
  }

  public int max_lanes = 0;

  public Profile max_lanes(int max_lanes) {
    this.max_lanes = max_lanes;
    return this;
  }

  // Maximum allowed time after race start after which results become overdue.
  // 0 = wait forever
  public long max_running_time_ms = 11000;

  public Profile max_running_time_ms(long max_running_time_ms) {
    this.max_running_time_ms = max_running_time_ms;
    return this;
  }

  // Defer reset and lane masking after a heat finishes to this many ms.
  // after a heat
  public long display_hold_time_ms = 10000;

  public Profile display_hold_time_ms(long display_hold_time_ms) {
    this.display_hold_time_ms = display_hold_time_ms;
    return this;
  }

  public static class PortParams {
    public int baudRate;
    public int dataBits;
    public int stopBits;
    public int parity;

    public PortParams(int baudRate, int dataBits, int stopBits, int parity) {
      this.baudRate = baudRate;
      this.dataBits = dataBits;
      this.stopBits = stopBits;
      this.parity = parity;
    }
  }
  public PortParams portParams;

  public Profile params(int baudRate, int dataBits, int stopBits, int parity) {
    portParams = new PortParams(baudRate, dataBits, stopBits, parity);
    return this;
  }

  public static class RemoteStart {
    public boolean has_remote_start;
    public String command;

    public RemoteStart(boolean has_remote_start, String command) {
      this.has_remote_start = has_remote_start;
      this.command = command;
    }
  }

  public RemoteStart remote_start;

  public Profile remote_start(boolean has_remote_start, String command) {
    remote_start = new RemoteStart(has_remote_start, command);
    return this;
  }

  public Profile remote_start(String command) {
    return remote_start(true, command);
  }

  public static class CommandSequence {
    public String[] commands;

    public CommandSequence(String... commands) {
      this.commands = commands;
    }
  }

  public static class Prober {
    public CommandSequence pre_probe;
    public String probe;
    public String[] response_patterns;

    public Prober(CommandSequence pre_probe, String probe,
                  String[] response_patterns) {
      this.pre_probe = pre_probe;
      this.probe = probe;
      this.response_patterns = response_patterns;
    }

    public Prober(String probe, String... response_patterns) {
      this(null, probe, response_patterns);
    }
  }

  // No prober => not detectable
  public Prober prober;

  public Profile prober(CommandSequence pre_probe, String probe,
                        String... response_patterns) {
    prober = new Prober(pre_probe, probe, response_patterns);
    return this;
  }

  public Profile prober(String probe, String... response_patterns) {
    return prober(null, probe, response_patterns);
  }

  public CommandSequence setup;
  public ArrayList<Query> setup_queries = new ArrayList<Query>();

  public Profile setup(String c) {
    setup = new CommandSequence(c);
    return this;
  }

  public Profile setup(String... commands) {
    setup = new CommandSequence(commands);
    return this;
  }

  public Profile setup(String query, Detector... detectors) {
    setup_queries.add(new Query(query, detectors));
    return this;
  }

  public static class Detector {
    public String pattern_string;
    public Detector[] internal_detectors;
    public Event event;
    public int[] arg_indexes;

    public Detector(String pattern_string,
                    Detector[] internal_detectors,
                    Event event, int... arg_indexes) {
      this.event = event;
      this.internal_detectors = internal_detectors;
      this.pattern_string = pattern_string;
      this.arg_indexes = arg_indexes;
    }

    public Detector(String pattern_string, Event event, int... arg_indexes) {
      this(pattern_string, null, event, arg_indexes);
    }

    public Detector(String pattern_string, Event event) {
      this(pattern_string, event, null);
    }
  }
  public ArrayList<Detector> detectors = new ArrayList<Detector>();

  public Profile match(String pattern_string, Detector[] internal_detectors,
                       Event event, int... arg_indexes) {
    detectors.add(new Detector(pattern_string, internal_detectors, event,
                               arg_indexes));
    return this;
  }

  public Profile match(String pattern_string, Detector... internal_detectors) {
    return match(pattern_string, internal_detectors,
                 (Event) null, (int[]) null);
  }

  public Profile match(String pattern_string, Event event, int... arg_indexes) {
    return match(pattern_string, null, event, arg_indexes);
  }

  public Profile match(String pattern_string, Event event) {
    return match(pattern_string, event, null);
  }

  public static class Query {
    public String command;
    public Detector[] detectors;

    public Query(String command, Detector... detectors) {
      this.command = command;
      this.detectors = detectors;
    }
  }

  // GateWatcher sends an interrogate_gate_command and then relies on a
  // Detector matching the report of the gate state to trigger an event.
  public Query gate_watcher;

  public Profile gate_watcher(String interrogate_gate_command,
                              Detector... detectors) {
    gate_watcher = new Query(interrogate_gate_command, detectors);
    return this;
  }

  public static class HeatPreparation {
    public String unmask_command;
    public String mask_command;
    public char first_lane;
    public String reset_command;

    public HeatPreparation(String unmask_command, String mask_command,
                           char first_lane, String reset_command) {
      this.unmask_command = unmask_command;
      this.mask_command = mask_command;
      this.first_lane = first_lane;
      this.reset_command = reset_command;
    }
  }
  public HeatPreparation heat_prep;

  public Profile heat_prep(String unmask_command, String mask_command,
                           char first_lane, String reset_command) {
    heat_prep = new HeatPreparation(unmask_command, mask_command, first_lane,
                                    reset_command);
    return this;
  }

  public Profile heat_prep(String unmask_command, String mask_command,
                           char first_lane) {
    return heat_prep(unmask_command, mask_command, first_lane, null);
  }

  public Profile heat_prep(String reset_command) {
    return heat_prep(null, null, (char)0, reset_command);
  }

  public Map<Event, CommandSequence> custom_handlers
      = new TreeMap<Event, CommandSequence>();

  public Profile on(Event event, CommandSequence cmd) {
    custom_handlers.put(event, cmd);
    return this;
  }

  public Profile on(Event event, String... cmd) {
    return on(event, new CommandSequence(cmd));
  }

  public Map<StateMachine.State, CommandSequence> custom_poll_actions
      = new TreeMap<StateMachine.State, CommandSequence>();

  public Profile during(StateMachine.State state, CommandSequence commands) {
    custom_poll_actions.put(state, commands);
    return this;
  }

  public Profile during(StateMachine.State state, String... cmd) {
    return during(state, new CommandSequence(cmd));
  }
}
