'use script';

const REMOTE_LOG_DELTA_MS = 100;

class Logger {
  static url = 'post-timer-log.php';

  buffer = [];
  last_write = Date.now();

  do_logging = false;

  scope = "";

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
                         '\n   platform=' + navigator.platform +
                         '\n   vendor=' + navigator.vendor +
                         '\n   userAgent=' + navigator.userAgent
                        );
    this.flush();
  }

  serial_in(s) {
    if (this.do_logging) {
      this.write_decorated('S <-- ', s);
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
    if (this.do_logging) {
      this.write_decorated('!!!!! ', err.toString());
      if (err.hasOwnProperty('stack')) {
        this.write_decorated('!!!!! ', err.stack);
      }
    }
    console.error(err);
    $("#messages").prepend($("<p/>").text(err.toString()).prepend("<img src='img/trouble-tiny.png'/>"));
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
    var now = Date.now();
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
setInterval(function () { g_logger.poll(); }, 250);
