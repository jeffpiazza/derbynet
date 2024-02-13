'use strict';

// The Prober oversees the scan of serial ports, trying to find a matching timer profile.

const PRE_PROBE_SETTLE_TIME_MS = 2000;
const PROBER_RESPONSE_TIME_MS = 500;

class Prober {
  user_chosen_port = -1;
  user_chosen_profile = -1;

  // Returns false for no match, or the timer identifier
  async probe_one_profile(pw, prof) {
    var deadline = Date.now() + PROBER_RESPONSE_TIME_MS;
    await pw.write(prof.prober.probe);

    var ri = 0;
    var re = new RegExp(prof.prober.responses[ri]);
    var s;
    while ((s = await pw.next(deadline)) != null) {
      if (Flag.debug_serial.value) {
        g_logger.debug_msg('   prober sees <<' + describeString(s) + '>>');
      }
      if (re.test(s)) {
        ++ri;
        if (ri >= prof.prober.responses.length) {
          return s.replace("\x1b", "");
        }
        re = new RegExp(prof.prober.responses[ri]);
      }
    }

    if (Flag.debug_serial.value) {
      g_logger.debug_msg('   probe match failed for /' + prof.prober.responses[ri] + '/');
    }
    return false;
  }

  give_up = false;
  probe_cycle_underway = false;

  // Returns immediately if a scan is already underway, but will start a new one
  // if not.
  async probe_until_found() {
    if (this.probe_cycle_underway) {
      console.log('probe_until_found quits because probe cycle underway');
      return;
    }
    this.probe_cycle_underway = true;
    g_logger.probing_started();
    TimerProxy.destroy();
    $("#connected").text("Probe started");
    try {
      if (g_ports.length == 0) {
        try {
          await request_new_port();
        } catch (e) {
          // Likely DOMException, Must be handling a user gesture...
          console.log(e);
          return;
        }
      }

      while (!g_timer_proxy && !this.give_up) {
        await g_prober.probe();
      }
      if (g_timer_proxy) {
        Flag.apply_all();
        if (g_host_poller) {
          g_host_poller.offer_remote_start(g_timer_proxy.has_remote_start());
        }
      }
    } finally {
      this.probe_cycle_underway = false;
      this.give_up = false;  // For next time
    }
  }
  
  async probe() {
    var profiles = all_profiles();
    if (g_ports.length == 0) {
      g_ports = await navigator.serial.getPorts();
    }
    if (g_ports.length == 0) {
      await request_new_port();
    }
    update_ports_list();

    // If g_ports changes during scan (e.g. via "New Port" button), we want to start the probe over
    // rather than confuse the UI.
    var n_ports = g_ports.length;

    for (var porti in g_ports) {
      Gui.probe_port(-1);

      if (g_ports.length != n_ports || this.give_up) {
        break;
      }

      if (this.user_chosen_port >= 0 && this.user_chosen_port != porti) {
        continue;
      }

      Gui.probe_port(porti);
      var port = g_ports[porti];
      for (var profi in profiles) {
        Gui.probe_profile(-1);
        if (g_ports.length != n_ports || this.give_up) {
          break;
        }
        var prof = profiles[profi];

        if (this.user_chosen_profile >= 0) {
          if (this.user_chosen_profile != profi) {
            console.log('Skipping profile ' + profi + ' because user chose ' + this.user_chosen_profile);
            continue;
          } else {
            console.log('Trying user-chosen profile ' + this.user_chosen_profile);
          }
        } else if (!prof.hasOwnProperty('prober')) {
          console.log('Skipping ' + prof.name);
          continue;
        }

        var info = g_ports[porti].getInfo();
        var label = '';
        if (info.hasOwnProperty('usbProductId')) {
          label = 'USB x' +
            info.usbVendorId.toString(16).padStart(4, '0') + '/x' +
            info.usbProductId.toString(16).padStart(4, '0');
        } else {
          label = '(built-in port)';
        }

        console.log("Probing for " + prof.name + ' on port ' + porti + ' ' + label);
        g_logger.internal_msg("Probing for " + prof.name + ' on port ' + porti + ' ' + label);

        Gui.probe_profile(profi);

        var pw = new PortWrapper(port);
        pw.eol = prof.options.eol;
        try {
          var opened_ok = true;
          await pw.open(prof.params)
            .catch((e) => {
              if (e.name == 'InvalidStateError') {
                g_logger.internal_msg('Serial port #' + porti + ' ' + label + ' was already open.');
                pw.port_is_open = true;
              } else {
                console.error('portwrapper open failed:', e);
                g_logger.internal_msg('Caught exception trying to open port ' + porti + ' ' + label);
                g_logger.stacktrace(e);
                g_logger.internal_msg('name: ' + e.name + ', code: ' + e.code + ', message: ' + e.message);
                Gui.probe_port_trouble(porti);
                opened_ok = false;
              }
            });
          if (!opened_ok) {
            // Remove the bad port.
            g_ports.splice(porti, 1);
            // If we tried opening the port when user_chosen_port wasn't zero,
            // then user_chosen_port was the bad port, so set it to zero.
            this.user_chosen_port = 0;
            break;
          }

          var timer_id;
          if (!prof.hasOwnProperty('prober')) {
            timer_id = true;
          } else {
            if (prof.prober.hasOwnProperty('pre_probe')) {
              await pw.writeCommandSequence(prof.prober.pre_probe);
              await pw.drain(PRE_PROBE_SETTLE_TIME_MS);
            }
            timer_id = await this.probe_one_profile(pw, prof);
          }

          if (timer_id !== false) {
            this.on_successful_match(porti, profi, prof, timer_id);

            // Avoid closing pw on the way out:
            var pw0 = pw;
            pw = null;

            return TimerProxy.create(pw0, prof);
          }
        } finally {
          if (pw) {
            await pw.close();
          }
        }
        Gui.probe_profile(-1);
      }
      Gui.probe_port(-1);
    }
  }

  on_successful_match(porti, profi, prof, timer_id) {
    console.log('*    Matched ' + prof.name + '!');
    g_logger.internal_msg('IDENTIFIED ' + prof.name);

    Gui.prober_complete(porti, profi);

    var vid = '';
    var pid = '';
    var usb_info = g_ports[porti].getInfo();
    if (usb_info.hasOwnProperty('usbVendorId') && usb_info.hasOwnProperty('usbProductId')) {
      vid = usb_info.usbVendorId.toString(16).padStart(4, '0');
      pid = usb_info.usbProductId.toString(16).padStart(4, '0');
    }
    TimerEvent.sendAfterMs(1000, 'IDENTIFIED', [prof.name, prof.hasOwnProperty('prober'),
                                                timer_id, vid, pid]);
    $("#probe-button").prop('disabled', true);
  }
}

var g_prober = new Prober();
