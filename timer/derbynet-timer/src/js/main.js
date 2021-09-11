// TODO
//
// - "Include new port" button
// - Disable "Probe" button when a timer has been found
// - Clickable <li>
// - Instructions

$(function() {
  var profiles = all_profiles();
  for (var i = 0; i < profiles.length; ++i) {
    $("#profiles-list").append($("<li/>").text(profiles[i].name)
                               .toggleClass('undiscoverable', !profiles[i]?.prober)
                               .on('click', on_user_profile_selection));
  }
});

var g_standalone = false;

var g_host_poller;
var g_role_finder = new RoleFinder();
var g_timer_proxy;

var g_ports = [];

$(function() {
  console.log('Starting initial action');
  if (!('serial' in navigator)) {
    $("#no-serial-api").removeClass('hidden');
    show_modal("#opening_modal");
  } else {
    setTimeout(async function() {
      if (g_standalone) {
        return;
      }

      console.log('Calling getPorts()');

      g_ports = await navigator.serial.getPorts();
      update_ports_list();

      if (g_ports.length == 0) {
        // It won't work to make a requestPort call except via a user gesture, so prompt the user
        $("#serial-start").removeClass('hidden');
        show_modal("#opening_modal");
      } else {
        console.log("Starting initial probe for timer with existing ports");
        probe_for_timer();
      }
    }, 1000);
  }
});

// This is the on-click handler for the "start scan" button from the opening modal
function on_serial_start() {
  close_modal("#opening_modal");
  probe_for_timer();
}

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

async function update_ports_list() {
  $("#ports-list").empty();
  for (var i = 0; i < g_ports.length; ++i) {
    var info = g_ports[i].getInfo();
    var label;
    if (info.hasOwnProperty('usbProductId')) {
      label = 'USB ' + info.usbVendorId + '/' + info.usbProductId;
    } else {
      label = 'Built-in port';
    }
    $("#ports-list").append($("<li/>")
                            .text(label)
                            .on('click', on_user_port_selection));
  }
}

var g_user_chosen_port = -1;
function on_user_port_selection(event) {
  // "this" is the <li> clicked
  console.log('on_user_port_selection', event, this);
  g_user_chosen_port = $(this).index();
}

var g_user_chosen_profile = -1;
function on_user_profile_selection(event) {
  console.log('on_user_profile_selection', event, this);
  g_user_chosen_profile = $(this).index();
}

async function new_port() {
  try {
    await navigator.serial.requestPort();
    g_ports = await navigator.serial.getPorts();
    update_ports_list();
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
    await navigator.serial.requestPort().catch(() => {});
    g_ports = await navigator.serial.getPorts();
  }
  update_ports_list();

  for (var porti in g_ports) {
    $("#ports-list li").removeClass('probing')

    if (g_user_chosen_port >= 0 && g_user_chosen_port != porti) {
      continue;
    }
    
    $("#ports-list li").eq(porti).addClass('probing')

    var port = g_ports[porti];
    for (var profi in profiles) {
      $("#profiles-list li").removeClass('probing');
      var prof = profiles[profi];

      if (g_user_chosen_profile >= 0) {
        if (g_user_chosen_profile != profi) {
          console.log('Skipping profile ' + profi + ' because user chose ' + g_user_chosen_profile);
          continue;
        } else if (!prof.hasOwnProperty('prober')) {
          console.log('Forcing selection of user-chosen unprobable profile ' + g_user_chosen_profile);
          $("#ports-list li").eq(porti).removeClass('probing').addClass('chosen');
          $("#profiles-list li").eq(profi).removeClass('probing').addClass('chosen');
          return new TimerProxy(new PortWrapper(port), prof);
        } else {
          console.log('Trying user-chosen profile ' + g_user_chosen_profile);
        }
      } else if (!prof.hasOwnProperty('prober')) {
        console.log('Skipping ' + prof.name);
        continue;
      }

      console.log("Probing for " + prof.name + ' on port ' + porti);
      $("#profiles-list li").eq(profi).addClass('probing');

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
              $("#ports-list li").eq(porti).removeClass('probing').addClass('chosen');
              $("#profiles-list li").eq(profi).removeClass('probing').addClass('chosen');
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
    $("#ports-list li").removeClass('probing chosen');
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
      }
      console.log('onEvent: ' + event + ' ' + (args || []).join(','));
    }
  });
});

async function probe_for_timer() {
  $("#connected").text("Probe started");
  g_timer_proxy = await probe();
  if (!g_timer_proxy) {
    $("#connected").text("Probe failed.");
  } else if (!g_standalone) {
    g_host_poller = new HostPoller();
  }
}


// Host side
async function on_connect_button(event) {
  console.log('on_connect_button');
  event.preventDefault();
  var ui_url = $("#host-url").val() + '/action.php';
  if (ui_url != HostPoller.url) {
    console.log('on_connect_button trying role_finder');
    HostPoller.url = ui_url;
    await g_role_finder.find_roles();
    if (g_role_finder.roles.length > 0) {
      console.log('role_finder apparently succeeded, with ' + g_role_finder.roles.length + ' role(s).');
      $("#role-select").empty();
      for (var i = 0; i < g_role_finder.roles.length; ++i) {
        $("<option>").appendTo("#role-select").text(g_role_finder.roles[i].name);
      }
      mobile_select_refresh("#role-select");
    }
  }
  if (g_host_poller) {
    console.log('g_host_poller already exists.');
  } else if ($("#role-select").val()) {
    console.log('Trying to log in, with role:' +
                $("#role-select").val() + ', pwd:' + $("#host-password").val());
    $.ajax(HostPoller.url,
           {type: 'POST',
            data: {action: 'role.login',
                   name:  $("#role-select").val(),
                   password: $("#host-password").val()},
            success: function(data) {
              console.log(data);
              if (data.outcome.summary == 'success') {
                console.log('Login succeeded, creating host poller.');
                g_host_poller = new HostPoller();
              }
            },
           });
  }
}
$(function() { $("#host-side-form").on('submit', on_connect_button); });
