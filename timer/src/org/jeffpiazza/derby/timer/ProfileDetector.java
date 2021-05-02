package org.jeffpiazza.derby.timer;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import jssc.SerialPortException;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public class ProfileDetector implements SerialPortWrapper.Detector {
  public ProfileDetector(Profile.Detector config, boolean active) {
    this.pattern = Pattern.compile(config.pattern_string);
    this.event = config.event;
    if (config.internal_detectors != null) {
      this.internal_detectors = new ProfileDetector[config.internal_detectors.length];
      for (int i = 0; i < config.internal_detectors.length; ++i) {
        internal_detectors[i] = new ProfileDetector(config.internal_detectors[i]);
      }
    }
    this.arg_indexes = config.arg_indexes;
    if (!active) {
      this.active_until = -1;
    }
  }

  public ProfileDetector(Profile.Detector config) {
    this(config, true);
  }

  private Pattern pattern;
  private ProfileDetector[] internal_detectors;
  private Event event;
  private int[] arg_indexes;

  // Named detectors are inactive (can't trigger) unless specifically activated,
  // normally to capture an expected response from a command.
  private long active_until = 0;

  public void activateFor(long ms) {
    active_until = System.currentTimeMillis() + ms;
  }

  @Override
  public String apply(String line) throws SerialPortException {
    if (!(active_until == 0 || System.currentTimeMillis() <= active_until)) {
      return line;
    }
    Matcher m = pattern.matcher(line);
    if (m.find()) {
      applyInternalDetectors(m);
      String[] args = new String[arg_indexes == null ? 0 : arg_indexes.length];
      if (arg_indexes != null) {
        for (int i = 0; i < arg_indexes.length; ++i) {
          args[i] = m.group(arg_indexes[i]);
        }
      }
      if (event != null) {
        Event.trigger(event, args);
      }
      return line.substring(0, m.start()) + line.substring(m.end());
    }
    return line;
  }

  protected void applyInternalDetectors(Matcher m) throws SerialPortException {
    if (internal_detectors != null) {
      String s = m.group();
      boolean match_more = true;
      while (match_more) {
        match_more = false;
        for (ProfileDetector d : internal_detectors) {
          String s2 = d.apply(s);
          if (s != s2) {  // Intentional pointer comparison
            match_more = true;
            s = s2;
            break;
          }
        }
      }
    }
  }
}
