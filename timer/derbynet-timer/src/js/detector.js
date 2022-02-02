'use strict';

// A detector is a regex that triggers an event upon matching.  The generated
// event may include strings picked up from the match.  In some cases, instead
// of (or in addition to) generating an event after a match, the detector may
// try applying some "internal" detectors to the parse the matched string into
// separate events.

class Detector {
  matcher;  // { pattern, event, arg_indexes, internal_detectors }
  active_until;

  regex;
  internal_detectors;

  constructor(matcher) {
    this.matcher = matcher;
    this.active_until = 0;
    this.regex = new RegExp(matcher.pattern);
    this.internal_detectors = [];
    var internal = matcher.internal_detectors || [];
    for (var i = 0; i < internal.length; ++i) {
      this.internal_detectors.push(new Detector(internal[i]));
    }
  }

  activateFor(ms) {
    this.active_until = Date.now() + ms;
  }
  activateUntil(deadline) {
    this.active_until = deadline;
  }

  apply(line) {
    if (!(this.active_until == 0 || Date.now() <= this.active_until)) {
      return line;
    }
    var m = line.match(this.regex);
    if (m != null) {
      if (Flag.debug_serial.value) {
        g_logger.serial_match( m[0] );
      }
      this.applyInternalDetectors(m);
      var args = [];
      if (this.matcher.hasOwnProperty('args')) {
        for (var i = 0; i < this.matcher.args.length; ++i) {
          args.push(m[parseInt(this.matcher.args[i])]);
        }
      }
      if (this.matcher.hasOwnProperty('event')) {
        TimerEvent.send(this.matcher.event, args);
      }
      return line.substr(0, m.index) + line.substr(m.index + m[0].length);
    }

    return line;
  }

  applyInternalDetectors(m) {
    var s = m[0];
    var match_more = true;
    while (match_more) {
      match_more = false;
      for (var i = 0; i < this.internal_detectors.length; ++i) {
        var s2 = this.internal_detectors[i].apply(s);
        if (s != s2) {
          match_more = true;
          s = s2;
          break;
        }
      }
    }
  }
}
