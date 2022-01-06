'use strict';

// A PortWrapper reads from the serial port, dividing the input into lines and
// then applying pattern matchers to each line as it becomes available.  A line
// is considered complete either when a newline character is seen, or after a
// long-enough pause.  The pattern matchers are typically looking for news of
// lane or heat finishes, or possibly news about the start gate being open or
// closed.

const COMMAND_DRAIN_MS = 100;


class PortWrapper {
  port;
  constructor(port) {
    this.port = port;
    this.leftover = "";
    this.lines = [];
  }

  reader;
  writer;

  leftover;
  lines;
  last_char_received;

  // Some timers require a particular end-of-line marker after a command
  eol = "";

  encoder = new TextEncoder();

  async open(params) {
    // params: baud, data, stop, parity

    // The constructor was likely called with a promise rather than a resolved port
    this.port = await this.port;

    // - Wait for the port to open.
    await this.port.open({ baudRate: params.baud,
                           dataBits: params.data || 8,
                           stopBits: params.stop || 1,  // 1 or 2
                           parity: params.parity || "none" /* none, even, odd */
                           /* flowcontrol: 'none' or flowcontrol: 'hardware' */
                         });

    this.reader = this.port.readable.getReader();

    // readLoop returns a Promise that only resolves when the port gets closed
    this.readloop_closed = this.readLoop();
  }

  // https://wicg.github.io/serial/#close-method for alternative readUntilClosed
  async close() {
    if (this.writer) {
      await this.writer.abort();
    }
    if (this.reader) {
      await this.reader.cancel();
    }
    await this.readloop_closed;
  }

  async readLoop() {
    let utf8decoder = new TextDecoder();
    try {
      while (true) {
        const { value, done } = await this.reader.read();

        if (value) {
          this.last_char_received = Date.now();
          this.leftover += utf8decoder.decode(value);
          var cr;
          while ((cr = this.leftover.indexOf('\n')) >= 0) {
            this.enqueueLine(this.leftover.substr(0, cr));
            this.leftover = this.leftover.substr(cr + 1);
          }
        }

        if (done) {
          break;
        }
      }
    } catch (err) {
      g_logger.internal_msg('PortWrapper.readLoop exited because of caught ' + err);
      console.log('PortWrapper.readLoop exited because of caught ' + err);
      TimerEvent.send('LOST_CONNECTION', [true, "Read failure: " + err]);
    } finally {
      await this.reader.releaseLock();
      // Reader has been released, and cannot be used to cancel its previous owner stream
      this.reader = null;
    }
  
    if (this.port.readable) {
      await this.port.readable.cancel();
    }

    await this.port.close();
  }

  noticeContact() {
    this.last_char_received = Date.now();
  }

  async checkConnection() {
    var age = Date.now() - this.last_char_received;
    if (age > /*LOST_CONTACT_THRESHOLD*/2000) {
      g_logger.internal_msg('PortWrapper.checkConnection detects lost connection by inactivity');
      console.log('PortWrapper.checkConnection detects lost connection by inactivity');
      await this.close();
      TimerEvent.send('LOST_CONNECTION',
                      [true, "No response from timer in " + age + "ms."]);
      throw "PortWrapper says: connection lost";
    }
  }
  
  enqueueLine(line) {
    if (line.length > 0) {
      g_logger.serial_in(line);
    }
    line = this.applyDetectors(line);
    if (line.length > 0) {
      this.lines.push(line);
    }
  }
  
  detectors = [];
  applyDetectors(line) {
    line = line.trim();
    var match_more = (line.length > 0);
    while (match_more) {
      match_more = false;
      for (var i = 0; i < this.detectors.length; ++i) {
        var s2 = this.detectors[i].apply(line);
        if (line != s2) {
          line = s2;
          match_more = (line.length > 0);
          break;
        }
      }
    }
    return line;
  }
      

  async write(msg) {
    g_logger.serial_out(msg);
    if (this.port.writable) {
      this.writer = this.port.writable.getWriter();
      try {
        await this.writer.write(this.encoder.encode(msg + this.eol));
      } finally {
        this.writer.releaseLock();
        this.writer = null;
      }
    } else {
      console.log('** No writable stream for port');
    }
    return msg;
  }

  nextNoWait() {
    if (!this.reader) {
      throw 'Reader is closed';
    }
    if (this.lines.length > 0) {
      return this.lines.shift().trim();
    }
    if (this.leftover.length > 0 &&
        Date.now() - this.last_char_received > Flag.newline_expected_ms.value) {
      var s = this.applyDetectors(this.leftover);
      this.leftover = "";
      if (s.length > 0) {
        return s;
      }
    }
    return null;
  }

  async writeCommandSequence(seq) {
    for (var i = 0; i < seq.commands.length; ++i) {
      await this.write(seq.commands[i]);
      await this.drain();
    }
  }

  async next(deadline) {
    do {
      var s = this.nextNoWait();
      if (s != null) {
        return s;
      }

      await new Promise(r => setTimeout(r, 50));
    } while (Date.now() < deadline);

    return null;
  }

  async drain(ms = /*COMMAND_DRAIN_MS*/100) {
    await this.drainUntil(Date.now() + ms);
  }

  async drainUntil(deadline) {
    var s;
    while ((s = await this.next(deadline)) != null) {
    }
  }
}
