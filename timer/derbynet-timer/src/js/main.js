var g_clock_worker;
if (window.Worker) {
  g_clock_worker = new Worker('js/timer/clock-worker.js');
  g_clock_worker.onmessage = function(e) {
    var key = e.data.shift();
    switch (key) {
    case 'HEARTBEAT':
      g_host_poller && g_host_poller.heartbeat();
      break;
    case 'POLL_TIMER':
      g_timer_proxy && g_timer_proxy.poll_once();
      break;
    case 'EVENT':
      var event = e.data[0];
      var args = e.data[1];
      TimerEvent.trigger(event, args);
      break;
    case 'LOGGER':
      g_logger && g_logger.poll();
      break;
    default:
      console.error('Unrecognized message from clock-worker', key, e.data);
    }
  }
} else {
  // Insurance: don't crash if browser doesn't support web worker
  //  (but it won't support web serial api)
  g_clock_worker = {postMessage: function(data) { console.error('web workers not supported:', data); }};
}

$(function() {
  var profiles = all_profiles();
  for (var i = 0; i < profiles.length; ++i) {
    $("#profiles-list").append($("<li/>").text(profiles[i].name)
                               .toggleClass('undiscoverable', !profiles[i]?.prober)
                               .on('click', on_user_profile_selection));
  }
});

$(function() {
  try {
    // Once per second, broadcast a "timer is alive" message, so other tabs/windows won't
    // try to open a second instance or refresh this one.
    const bc = new BroadcastChannel('timer-alive');
    setInterval(function() {
      bc.postMessage({alive: true});
    }, 1000);

    // We can try to bring our window to the front, but it mostly doesn't work.
    const bc_focus = new BroadcastChannel('timer-focus');
    bc_focus.onmessage = function(ev) { window.focus(); }
  } catch (ex) {
    // BroadcastMessage isn't supported on all browsers
  }
});

// g_standalone gets set to true for standalone Electron version by inline
// script tag that follows the import of this file.
var g_standalone = false;

var g_role_finder = new RoleFinder();
var g_timer_proxy;

var g_ports = [];

$(function() {
  console.log('Starting initial action');
  if (false && !g_standalone) {  // TODO
    $("#port-button").addClass('hidden');
  }
  if (!('serial' in navigator)) {
    if (!window.isSecureContext) {
      var link = "https://" + window.location.hostname + window.location.pathname;
      $("#no-serial-api-http p a").prop('href', link).text(link);
      $("#no-serial-api-http").removeClass('hidden');
    } else {
      $("#no-serial-api").removeClass('hidden');
    }
    show_modal("#no-serial-api-modal");
  } else if (!g_standalone) {
    setTimeout(async function() {
      console.log('timeout upon entry: calling getPorts()');

      g_ports = await navigator.serial.getPorts();
      update_ports_list();

      if (g_ports.length == 0) {
        // Display a modal, asking for a user gesture
        show_modal("#need-gesture-modal");
      } else {
        on_scan_click();
      }
    }, 1000);
  }
});

// This is the handler for the button in the #need-gesture-modal, offered when
// there's nothing we can do until the user makes some gesture.
function on_gesture_click() {
  close_modal("#need-gesture-modal");
  on_scan_click();
}


function on_user_port_selection(event) {
  // "this" is the <li> clicked
  g_prober.user_chosen_port = $(this).index();
  $(this).siblings().removeClass('user-chosen');
  $(this).addClass('user-chosen');
}

function on_user_profile_selection(event) {
  g_prober.user_chosen_profile = $(this).index();
  $(this).siblings().removeClass('user-chosen');
  $(this).addClass('user-chosen');
}

async function request_new_port() {
  await new Promise((resolve, reject) => {
    $("#second_modal_background").fadeTo(200, 0.75, function() { resolve(1); });
  });
  try {
    var make_request = true;
    while (make_request) {
      make_request = g_standalone;
      // In browser, ask the user to pick one port.
      // In standalone, keep going until requestPort() throws (app will try to select every available port)
      await navigator.serial.requestPort().catch((e) => {
        // throw e;
        make_request = false;
      });
      g_ports = await navigator.serial.getPorts();
    }
  } finally {
    $("#second_modal_background").fadeOut(200);
  }
}

// "New Port" button
async function on_new_port_click() {
  // The "New Port" button forcibly requests a new port even if there are other
  // ports available, so the user gets the chance to choose
  await request_new_port();
  update_ports_list();
  g_prober.probe_until_found();
}


async function update_ports_list() {
  g_ports.sort((p1, p2) => {
    var p1_usb = p1.getInfo()?.usbProductId && true;
    var p2_usb = p2.getInfo()?.usbProductId && true;
    return p1_usb == p2_usb ? 0
      : p1_usb ? -1 : 1;
  });

  $("#ports-list li").slice(g_ports.length).remove();
  while ($("#ports-list li").length < g_ports.length) {
    // If ports have been added, the old ports may not be in the same positions,
    // so clear the CSS classes to avoid misleading UI.
    $("#ports-list li").removeClass('trouble probing chosen');
    $("#ports-list").append($("<li/>")
                            .on('click', on_user_port_selection));
  }
  for (var i = 0; i < g_ports.length; ++i) {
    var info = g_ports[i].getInfo();
    var label;
    if (info.hasOwnProperty('usbProductId')) {
      label = 'USB x' +
        info.usbVendorId.toString(16).padStart(4, '0') + '/x' +
        info.usbProductId.toString(16).padStart(4, '0');
    } else {
      label = 'Built-in port';
    }
    $("#ports-list li").eq(i).text(label);
  }
}

$(function() {
  var isOpen = false;
  TimerEvent.register({
    onEvent: function(event, args) {
      switch (event) {
      case 'GATE_OPEN':
        if (isOpen) return;
        isOpen = true;
        break;
      case 'GATE_CLOSED':
        if (!isOpen) return
        isOpen = false;
        break;
      case 'LOST_CONNECTION':
        console.log('Handling LOST_CONNECTION event.');
        $("#probe-button").prop('disabled', false);
        setTimeout(async function() {
          // TODO This conditional teardown call doesn't seem to happen.
          // Fortunately TimerProxy unregisters itself now, upon receiving a
          // LOST_CONNECTION event.
          if (g_timer_proxy) {
            // issue#187: after a lost connection, the timer proxy et al are
            // still registered for events, and will duplicate the effect of the
            // new timer proxy unless torn down.
            await g_timer_proxy.teardown();
            g_timer_proxy = null;
          }
          console.log('Starting probe after lost connection');
          await g_prober.probe_until_found();
          console.log('Probe complete: g_timer_proxy=', g_timer_proxy);
        }, 0);
        break;
      }
      console.log('onEvent: ' + event + ' ' + (args || []).join(','));
    }
  });
});

// The "Scan" button
async function on_scan_click() {
  g_prober.probe_until_found();
}


// Host side
async function on_connect_button(event) {
  event.preventDefault();
  var ui_url = $("#host-url").val() + '/action.php';
  if (!(ui_url.startsWith("http://") || ui_url.startsWith("https://"))) {
    ui_url = "http://" + ui_url;
  }
  if (ui_url != HostPoller.url) {
    HostPoller.url = ui_url;
    await g_role_finder.find_roles();
    if (g_role_finder.roles.length > 0) {
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
                $("#host-status").prop('src', "img/status/ok.png");
                if (g_timer_proxy) {
                  g_host_poller.offer_remote_start(g_timer_proxy.has_remote_start());
                }
              }
            },
           });
  }
}
$(function() { $("#host-side-form").on('submit', on_connect_button); });
