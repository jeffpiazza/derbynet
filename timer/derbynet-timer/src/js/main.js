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
    $("#no-serial-api").removeClass('hidden');
    show_modal("#no-serial-api-modal");
  } else {
    setTimeout(async function() {
      if (g_standalone) {
        return;
      }

      console.log('Calling getPorts()');

      g_ports = await navigator.serial.getPorts();
      update_ports_list();

      on_scan_click();
    }, 1000);
  }
});

if (!g_standalone) {
  $(window).bind("beforeunload", function(event) {
    if (g_timer_proxy) {
      // Chrome ignores the prompt and substitutes its own generic message.  Gee, thanks.
      show_modal("#leaving_modal");
      setTimeout(function() { close_modal("#leaving_modal"); }, 10000);
      var prompt =
          "Leaving this page will disconnect the timer.  Are you sure you want to exit?";
      event.preventDefault();
      event.returnValue = prompt;
      return prompt;
    }
  });
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
    await navigator.serial.requestPort().catch(() => {});
  } finally {
    $("#second_modal_background").fadeOut(200);
  }
  g_ports = await navigator.serial.getPorts();
}

async function on_new_port_click() {
  await request_new_port();
  update_ports_list();
  g_prober.probe_until_found();
}


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
        beak;
      case 'LOST_CONNECTION':
        $("#probe-button").prop('disabled', false);
        setTimeout(async function() { g_timer_proxy = await g_prober.probe_until_found(); }, 0);
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
