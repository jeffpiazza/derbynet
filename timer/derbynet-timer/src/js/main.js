// TODO
//
// - "Include new port" button
// - Disable "Probe" button when a timer has been found
// - Clickable <li>
// - Instructions

$(function() {
  if (!('serial' in navigator)) {
    $("#no-serial-api").removeClass('hidden');
  }
});

$(function() {
  var profiles = all_profiles();
  for (var i = 0; i < profiles.length; ++i) {
    $("#profiles-list").append($("<li/>").text(profiles[i].name)
                               .toggleClass('undiscoverable', !profiles[i]?.prober));
  }
});

var g_host_poller;
var g_timer_proxy;

if (false) {
$(window).bind("beforeunload", function(event) {
  console.log('BeforeUnload fires');
  if (g_timer_proxy) {
    // Chrome ignores the prompt and substitutes its own generic message.  Gee, thanks.
    var prompt =
        "Leaving this page will disconnect the timer.  Are you sure you want to exit?";
    event.preventDefault();
    event.returnValue = prompt;
    return prompt;
  }
});
}


const PRE_PROBE_SETTLE_TIME_MS = 2000;
const PROBER_RESPONSE_TIME_MS = 500;
  
async function exercise() {

  var port = await navigator.serial.requestPort();
  
  var pw = new PortWrapper(port);

  await pw.open({baud: 9600});

  await pw.write("RV");

  setTimeout(async function() {
    var s;
    while ((s = pw.nextNoWait()) != null) {
      console.log('cleanup: ' + s + " (" + s.length + ")");
    }
    await pw.close();
  }, 1000);

  var deadline = Date.now() + 3000;
  try {
    var s;
    while ((s = await pw.next(deadline)) != null) {
      console.log('readNoWait: ' + s + " (" + s.length + ")");
    }
    console.log('Deadline passed');
  } catch (err) {
    console.log('Exercise catch');
    if (err != 'Reader is closed') {
      throw err;
    }
  } finally {
    console.log('All done');
  }
}

g_ports = [];

async function new_port() {
  try {
    g_ports = [await navigator.serial.requestPort()];
    $("#probe-button").prop('disabled', false);
    g_timer_proxy = await probe();
  } catch (err) {
  }
}


async function probe() {
  var profiles = all_profiles();
  if (g_ports.length == 0) {
    g_ports = await navigator.serial.getPorts();
  }
  if (g_ports.length == 0) {
    await navigator.serial.requestPort();
    g_ports = await navigator.serial.getPorts();
  }
  for (var porti in g_ports) {
    var port = g_ports[porti];
    for (var profi in profiles) {
      var prof = profiles[profi];
      if (!prof.hasOwnProperty('prober')) {
        console.log('Skipping ' + prof.name);
        continue;
      }

      console.log("Probing for " + prof.name + ' on port ' + porti);
      $("#connected-detail").text("Probing for " + prof.name + " on port " + porti);
      $("#profiles-list li").removeClass('probing');
      $($("#profiles-list li")[profi]).addClass('probing');

      var pw = new PortWrapper(port);
      try {
        await pw.open(prof.params);
        if (prof.prober.hasOwnProperty('pre_probe')) {
          await pw.writeCommandSequence(prof.prober.pre_probe);
          await pw.drain(PRE_PROBE_SETTLE_TIME_MS);
        }

        var deadline = Date.now() + PROBER_RESPONSE_TIME_MS;
        await pw.write(prof.prober.probe);

        var ri = 0;
        var re = new RegExp(prof.prober.responses[ri]);
        var s;
        while ((s = await pw.next(deadline)) != null) {
          if (re.test(s)) {
            ++ri;
            if (ri >= prof.prober.responses.length) {
              console.log('*    Matched ' + prof.name + '!');
              $($("#profiles-list li")[profi]).removeClass('probing').addClass('chosen')
              TimerEvent.sendAfterMs(1000, 'IDENTIFIED', [prof.name, s]);
              $("#probe-button").prop('disabled', true);
              // Avoid closing pw on the way out:
              var pw0 = pw;
              pw = null;
              return new TimerProxy(pw0, prof);
            }
            re = new RegExp(prof.prober.responses[ri]);
          }
        }
      } finally {
        if (pw) {
          await pw.close();
        }
      }
      $("#profiles-list li").removeClass('probing chosen');
    }
  }
}

$(function() {
  var isOpen = false;
  TimerEvent.register({
    onEvent: function(event, args) {
      if (event == 'GATE_OPEN') {
        if (isOpen) return;
        isOpen = true;
      }
      if (event == 'GATE_CLOSED') {
        if (!isOpen) return
        isOpen = false;
      }
      if (event == 'IDENTIFIED') {
        $("#connected").text('CONNECTED: ' + args[0]);
        $("#connected-detail").text(args[1]);
      }
      console.log('onEvent: ' + event + ' ' + (args || []).join(','));
      $("#last_event").text(event + ' ' + (args || []).join(','));
    }
  });
});

async function probe_for_timer() {
  $("#connected").text("Probe started");
  g_timer_proxy = await probe();
  if (!g_timer_proxy) {
    $("#connected").text("Probe failed.");
  } else {
    g_host_poller = new HostPoller();
  }
}
