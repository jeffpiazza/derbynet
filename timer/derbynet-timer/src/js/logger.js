'use script';

const REMOTE_LOG_DELTA_MS = 100;

class Logger {
  static url = 'post-timer-log.php';

  buffer = [];
  last_write = Date.now();

  do_logging = false;

  scope = "";

  first_stacktrace;  // Records the argument to the very first call to stacktrace,
                     // so it can be reported when do_logging becomes true.
  first_probe_start_time = -1;  // Date.now() when probing first started

  constructor(do_logging) {
    this.set_remote_logging(do_logging);
  }
  
  set_remote_logging(do_logging) {
    if (do_logging && !this.do_logging) {
      Logger.set_url(HostPoller.url);
      this.do_logging = true;
      this.start_log();
    }
    this.do_logging = do_logging;
  }

  static set_url(url) {
    Logger.url = url.replace(/action.php$/, 'post-timer-log.php');
  }

  // 
  start_log() {
    this.write_decorated(' HELLO',
                         '\n   version=' + g_version.branch + '-' + g_version.revision +
                                 ', ' + g_version.date +
                         '\n   platform=' + navigator.platform +
                         '\n   vendor=' + navigator.vendor +
                         '\n   userAgent=' + navigator.userAgent +
                         '\n   page loaded=' + ((Date.now() - g_page_loaded)/1000) + 's ago' +
                         '\n   scan started=' + (this.first_probe_start_time < 0 ? 'Not yet'
                                                 : ((Date.now() - this.first_probe_start_time)/1000)
                                                   + 's ago')
                        );
    if (this.first_stacktrace) {
      this.internal_msg('Stacktrace recorded earlier:');
      this.stacktrace(this.first_stacktrace);
    }
    this.flush();
  }

  probing_started() {
    if (this.first_probe_start_time < 0) {
      this.first_probe_start_time = Date.now();
    }
  }

  serial_in(s) {
    if (this.do_logging) {
      this.write_decorated('S <-- ', s);
    }
  }

  serial_in_inferred(s) {
    if (this.do_logging) {
      this.write_decorated('S <-* ', s);
    }
  }

  serial_out(s) {
    if (this.do_logging) {
      this.write_decorated('S --> ', s);
    }
  }

  serial_match(s) {
    if (this.do_logging) {
      this.write_decorated('S /-/ ', s);
    }
  }

  internal_msg(s) {
    if (this.do_logging) {
      this.write_decorated('* INT ', s);
    }
  }

  debug_msg(s) {
    if (this.do_logging) {
      this.write_decorated('* DBG ', s);
    }
  }

  host_in(s) {  // Message from host
    if (this.do_logging) {
      this.write_decorated('H <-- ', s);
    }
  }

  stacktrace(err) {
    if (!this.first_stacktrace) {
      this.first_stacktrace = err;
    }
    if (this.do_logging) {
      this.write_decorated('!!!!! ', err.toString());
      if (err.hasOwnProperty('stack')) {
        this.write_decorated('!!!!! ', err.stack);
      }
    }
    Gui.trouble_message(err);
  }

  write_decorated(key, s) {
    var d = new Date();
    this.write_raw('+' + /* d.getHours().toString().padStart(2, '0') +
                   ':' + d.getMinutes().toString().padStart(2, '0') +
                   ':' + */ d.getSeconds().toString().padStart(2, '0') +
                   '.' + d.getMilliseconds().toString().padStart(3, '0') +
                   key + s);
  }

  write_raw(s) {
    if (this.do_logging == 0) {
      return;
    }
    this.buffer.push(s);
  }

  poll() {
    if (this.buffer.length > 0) {
      this.flush();
    }
  }
  
  flush() {
    var p = this.buffer.join('\n') + '\n';
    this.buffer = [];
    this.last_write = Date.now();
    $.ajax(Logger.url,
           {type: 'POST',
            data: p,
           });
  }

}

var g_logger = new Logger(false);
