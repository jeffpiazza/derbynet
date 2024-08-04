package org.jeffpiazza.derby.timer;

// A Profile describes a particular model timer.
//
// {name:
//  key:
//  params: {baud, data, stop, parity}
//  options: {max_lanes, eol, max_running_times_ms}
//  prober: {pre_probe, probe, responses}
//  setup: {commands: [c1, c2, ...]},
//  setup_queries: [{command, matchers: [...]}, ...]
//  matchers: [{pattern, internal, event, args}, ...]
//  gate_watcher: {command, matchers[...]}
//  heat_prep: {unmask, mask, lane, reset}
//  on: { <event>: {commands: [c1, c2, ...]}, ... }
//  poll: { <state>: , {commands: [c1, c2, ...]}, ... }
//  remote_start: {has_remote_start, commands: [c1, c2, ...]}
// }
//
//  options.max_lanes gives the maximum number of lanes for this kind of timer;
//   it's used to interpret lane masks if the actual number of lanes is not
//   known.
//  options.eol is an end-of-line character to be appended to each command
//  options.max_running_times_ms sets an upper bound on how long a heat can run
//   before results are overdue
//  options.diplay_hold_until_ms tells how long after a heat ends can the timer
//   be reset (to allow timer display to be seen).
//
//  A matcher includes a regex pattern and an event to generate.  Args tells
//   the matched group(s) that should be passed as arguments with the event.
//   internal, if present, is another list of matchers that should be repeatedly
//   applied to the string that matched the outer matcher's pattern.
//
//  prober gives a command (probe) and response regexes to identify the timer,
//   if such identification is possible.  pre_probe, if present, is a command to
//   reset or settle the timer before probing.
//
//  setup is a sequence of commands to set up the timer once it's been identified.
//  setup_queries is a collection of commands and matchers to interrogate the
//   timer for details (like number of lanes), if such interrogation is possible.
//
//  A gate_watcher is present if the starting gate state can be determined by
//   polling; the command string gets sent to the timer and the matchers are
//   applied to any responses.
//
//  heat_prep describes how to prepare the timer for the next heat.  If the
//   timer supports lane masking, then the fields unmask, mask,
//   and lane describe how to construct the commands to set the lane mask.
//   reset is the command string to reset the timer, if the timer can be reset
//   from the host.
//
//  on and poll describe additional commands to be sent when a given event occurs
//   (on) or whenever the timer is in a given state (poll).
//
//  remote_start, if present, indicates that the timer may support a remote
//   start gate release.  The has_remote_start property, if present, indicates
//   whether this specific timer supports remote start.  The commands property
//   gives the actual command sequence to open the start gate.
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;
import org.json.JSONArray;
import org.json.JSONObject;

public class Profile {
  public String name;
  public String key;

  private Profile(String name, String key) {
    this.name = name;
    this.key = key;
  }

  public static Profile forTimer(String name, String key) {
    return new Profile(name, key);
  }

  public Profile rename(String name, String key) {
    this.name = name;
    this.key = key;
    return this;
  }

  public static class Options {
    public int max_lanes = 0;
    // Some timers require a particular end-of-line character or sequence
    public String eol = "";
    // Some timers only report heat finished, but not whether the start gate
    // is open or closed.
    public boolean gate_state_is_knowable = true;

    public JSONObject toJSON() {
      return new JSONObject()
          .put("eol", eol)
          .put("max_lanes", max_lanes)
          .put("gate_state_is_knowable", gate_state_is_knowable);
    }
  }
  public Options options = new Options();

  public Profile end_of_line(String eol) {
    options.eol = eol;
    return this;
  }

  public Profile max_lanes(int max_lanes) {
    options.max_lanes = max_lanes;
    return this;
  }

  public Profile gate_state_is_knowable(boolean gate_state_is_knowable) {
    options.gate_state_is_knowable = gate_state_is_knowable;
    return this;
  }

  public static class PortParams {
    public int baud;
    public int data;
    public int stop;
    public int parity;

    public PortParams(int baudRate, int dataBits, int stopBits, int parity) {
      this.baud = baudRate;
      this.data = dataBits;
      this.stop = stopBits;
      this.parity = parity;
    }

    public JSONObject toJSON() {
      return new JSONObject()
          .put("baud", baud)
          .put("data", data)
          .put("stop", stop)
          .put("parity", parity);
    }
  }
  public PortParams params;

  public Profile params(int baudRate, int dataBits, int stopBits, int parity) {
    params = new PortParams(baudRate, dataBits, stopBits, parity);
    return this;
  }

  public static class RemoteStart {
    public String has_remote_start;
    public String command;

    public RemoteStart(String has_remote_start, String command) {
      this.has_remote_start = has_remote_start;
      this.command = command;
    }

    public JSONObject toJSON() {
      return new JSONObject()
          .put("has_remote_start", has_remote_start)  // Name of a RuntimeCondition
          .put("command", command);
    }
  }

  public RemoteStart remote_start;

  public Profile remote_start(String has_remote_start, String command) {
    remote_start = new RemoteStart(has_remote_start, command);
    return this;
  }

  public Profile remote_start(String command) {
    return remote_start(null, command);
  }

  public static class CommandSequence {
    public String[] commands;

    public CommandSequence(String... commands) {
      this.commands = commands;
    }

    public JSONObject toJSON() {
      return new JSONObject()
          .put("commands", new JSONArray(commands));
    }
  }

  public static class Prober {
    public CommandSequence pre_probe;
    public String probe;
    public String[] responses;

    public Prober(CommandSequence pre_probe, String probe,
                  String[] response_patterns) {
      this.pre_probe = pre_probe;
      this.probe = probe;
      this.responses = response_patterns;
    }

    public Prober(String probe, String... response_patterns) {
      this(null, probe, response_patterns);
    }

    public JSONObject toJSON() {
      JSONObject j = new JSONObject()
          .put("probe", probe)
          .put("responses", new JSONArray(responses));
      if (pre_probe != null) {
        j.put("pre_probe", pre_probe.toJSON());
      }
      return j;
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
    public String pattern;
    public Detector[] internal_detectors;
    public Event event;
    public int[] args;

    public Detector(String pattern_string,
                    Detector[] internal_detectors,
                    Event event, int... arg_indexes) {
      this.event = event;
      this.internal_detectors = internal_detectors;
      this.pattern = pattern_string;
      this.args = arg_indexes;
    }

    public Detector(String pattern_string, Event event, int... arg_indexes) {
      this(pattern_string, null, event, arg_indexes);
    }

    public Detector(String pattern_string, Event event) {
      this(pattern_string, event, null);
    }

    public JSONObject toJSON() {
      return new JSONObject()
          .put("pattern", pattern)
          .putOpt("internal", internal_detectors == null ? null
                              : detectorsToJSON(Arrays.
                      asList(internal_detectors)))
          .putOpt("event", event == null ? null : event.toString())
          .putOpt("args",
                  args == null || args.length == 0 ? null
                  : new JSONArray(args));
    }
  }
  public ArrayList<Detector> matchers = new ArrayList<Detector>();

  public Profile match(String pattern_string, Detector[] internal_detectors,
                       Event event, int... arg_indexes) {
    matchers.add(new Detector(pattern_string, internal_detectors, event,
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
    public Detector[] matchers;

    public Query(String command, Detector... matchers) {
      this.command = command;
      this.matchers = matchers;
    }

    public JSONObject toJSON() {
      return new JSONObject()
          .put("command", command)
          .put("matchers", detectorsToJSON(Arrays.asList(matchers)));
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
    public String unmask;
    public String mask;
    public char lane;
    public String reset;

    public HeatPreparation(String unmask_command, String mask_command,
                           char first_lane, String reset_command) {
      this.unmask = unmask_command;
      this.mask = mask_command;
      this.lane = first_lane;
      this.reset = reset_command;
    }

    public JSONObject toJSON() {
      return new JSONObject()
          .putOpt("unmask", unmask)
          .putOpt("mask", mask)
          .putOpt("lane", mask == null ? null : lane)
          .putOpt("reset", reset);
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
    return heat_prep(null, null, (char) 0, reset_command);
  }

  public Map<Event, CommandSequence> on
      = new TreeMap<Event, CommandSequence>();

  public Profile on(Event event, CommandSequence cmd) {
    on.put(event, cmd);
    return this;
  }

  public Profile on(Event event, String... cmd) {
    return on(event, new CommandSequence(cmd));
  }

  public static class StatePoller {
    public String condition;
    public CommandSequence commands;

    public StatePoller(String condition, CommandSequence commands) {
      this.condition = condition;
      this.commands = commands;
    }
    public JSONObject toJSON() {
      return new JSONObject()
          .put("condition", condition)
          .put("commands", commands.toJSON())
          ;
    }
  }

  public Map<StateMachine.State, StatePoller> poll
      = new TreeMap<StateMachine.State, StatePoller>();

  public Profile during(StateMachine.State state, StatePoller poller) {
    poll.put(state, poller);
    return this;
  }
  public Profile during(StateMachine.State state, String condition, CommandSequence commands) {
    return during(state, new StatePoller(condition, commands));
  }
  public Profile during(StateMachine.State state, String condition, String... commands) {
    return during(state, condition, new CommandSequence(commands));
  }

  protected static JSONArray queriesToJSON(List<Query> queries) {
    JSONArray array = new JSONArray();
    for (Query q : queries) {
      array.put(q.toJSON());
    }
    return array;
  }

  protected static JSONArray detectorsToJSON(List<Detector> detectors) {
    JSONArray array = new JSONArray();
    for (Detector d : detectors) {
      array.put(d.toJSON());
    }
    return array;
  }

  protected static JSONObject handlersToJSON(
      Map<Event, CommandSequence> handlers) {
    JSONObject obj = new JSONObject();
    for (Entry<Event, CommandSequence> entry : handlers.entrySet()) {
      obj.put(entry.getKey().toString(), entry.getValue().toJSON());
    }
    return obj;
  }

  protected static JSONObject pollersToJSON(
      Map<StateMachine.State, StatePoller> pollers) {
    JSONObject obj = new JSONObject();
    for (Entry<StateMachine.State, StatePoller> entry : pollers.entrySet()) {
      obj.put(entry.getKey().toString(), entry.getValue().toJSON());
    }
    return obj;
  }

  public JSONObject toJSON() {
    return new JSONObject()
        .put("name", name)
        .put("key", key)
        .put("options", options.toJSON())
        .put("params", params.toJSON())
        .putOpt("remote_start", remote_start == null ? null
                                : remote_start.toJSON())
        .putOpt("prober", prober == null ? null : prober.toJSON())
        .putOpt("setup", setup == null ? null : setup.toJSON())
        .putOpt("setup_queries", setup_queries.size() == 0 ? null
                                 : queriesToJSON(setup_queries))
        .put("matchers", detectorsToJSON(matchers))
        .put("gate_watcher", gate_watcher == null ? null : gate_watcher.toJSON()).
        put("heat_prep", heat_prep == null ? null : heat_prep.toJSON())
        .putOpt("on", on.size() == 0 ? null : handlersToJSON(on))
        .putOpt("poll", poll.size() == 0 ? null : pollersToJSON(poll));
  }
}
