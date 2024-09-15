'use strict';

function on_lane_count_change() {
  var nlanes = $("#n-lanes").val();
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'settings.write',
                 'n-lanes': nlanes}
         });
  var mask = $("#unused-lane-mask").val();
  $("table#lanes tr.lane").remove();
  if (nlanes <= 0) {
    nlanes = 1;
  }
  for (var i = 0; i < nlanes; ++i) {
    var bit = 1 << i;
    $("table#lanes").append(
      $("<tr class='lane' data-lane='" + (i + 1) + "'></tr>")
        .append("<td>" + (i + 1) + "</td>")
        .append($("<td data-bit='" + bit + "'></td>")
                .append($("<img src='img/car-60.png'/>").toggleClass('hidden', (mask & bit) != 0)))
        .append("<td class='time'></td>")
        .append("<td class='place'></td>"));
  }

  // In case the lane count decreased, clear any higher-order bits as they're no
  // longer meaningful
  $("#unused-lane-mask").val(mask & ~(-1 << nlanes));
  
  $("table#lanes td[data-bit]").on('click', on_mask_click);
}

$(function() {
  on_lane_count_change();
  $("#n-lanes").on('keyup mouseup', on_lane_count_change);
});

function on_reverse_lanes_change() {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'settings.write',
                 'reverse-lanes': $("#reverse-lanes").is(':checked') ? 1 : 0,
                 'reverse-lanes-checkbox': 1}
        });
}
$(function() { $("#reverse-lanes").on('change', on_reverse_lanes_change); });

// "Timer Settings" button clicked: fetch the timer settings from the server and
// then display the modal to change them.
function handle_timer_settings_button() {
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'timer.settings'},
          success: function(data) {
            open_timer_settings_modal(data);
          }});
}

function open_timer_settings_modal(data) {
  var ports = [];
  if (data.hasOwnProperty('ports')) {
    ports = data.ports.split(',');
  }

  $("#timer_settings_port .mselect").toggleClass('hidden', ports.length == 0);
  if (ports.length != 0) {
    $("#timer_settings_port select").empty();
    $("#timer_settings_port select").append("<option value='' selected='selected'>Auto Port</option>");
    for (var i = 0; i < ports.length; ++i) {
      $("<option/>")
        .attr('value', ports[i])
        .text(ports[i])
        .appendTo($("#timer_settings_port select"));
    }
  }

  var devices = data.devices;
  $("#timer_settings_device .mselect").toggleClass('hidden', devices.length == 0);
  if (devices.length != 0) {
    $("#timer_settings_device select").empty();
    $("#timer_settings_device select").append("<option value='' selected='selected'>Auto Device</option>");
    for (var i = 0; i < devices.length; ++i) {
      $("<option/>")
        .attr('value', devices[i].name)
        .text(devices[i].description)
        .appendTo($("#timer_settings_device select"));
    }
  }
  
  var flags = data.flags;
  $("#timer_settings_modal_flags").empty();
  for (var i = 0; i < flags.length; ++i) {
    var f = flags[i];
    $("#timer_settings_modal_flags").append(
      $("<tr/>")
        .append($("<td/>").text(f.name))
        .append($("<td/>").text(f.description))
        .append(make_flag_control(f, $("<td/>")))
    );
  }

  flipswitch($("#timer_settings_modal").find("input[type='checkbox']"));
  mobile_select_refresh($("#timer_settings_modal select"));

  show_modal("#timer_settings_modal", function(event) {});
}

function on_port_change(evt) {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'timer.assign-port',
                 port: $("#timer_settings_port select")
                        .find('option:selected').attr('value')
                }});
}
$(function() { $("#timer_settings_port select").on('change', on_port_change); });

function on_device_change(evt) {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'timer.assign-device',
                 device: $("#timer_settings_device select")
                           .find('option:selected').attr('value')
                }});
}
$(function() { $("#timer_settings_device select").on('change', on_device_change); });

function on_flag_change_bool(evt) {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'timer.assign-flag',
                 flag: $(evt.target).attr('data-flag'),
                 value: $(evt.target).is(':checked') ? 'true' : 'false'
                }});
  close_modal("#timer_settings_modal");
}

function on_flag_input_text(evt) {
  var target = $(evt.target);
  target.closest('td').find('div.controls')
    .toggleClass('hidden', target.val() == target.attr('original'));
}
function on_flag_check(evt) {
  var target = $(evt.target);
  var input = target.closest('td').find('input[type="text"]');
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'timer.assign-flag',
                 flag: input.attr('data-flag'),
                 value: input.val()
                }});
  close_modal("#timer_settings_modal");
}
function on_flag_cross(evt) {
  var target = $(evt.target);
  var input = target.closest('td').find('input[type="text"]');
  input.val(input.attr('original'));
  on_flag_input_text({target: input});
}

function make_flag_control(f, td) {
  var t = f.type;
  if (t == 'bool') {
    td.append($('<input type="checkbox" class="flipswitch"'
                + (f.value != 'false' ? ' checked="checked"' : '')
                + '/>')
              .attr('data-flag', f.name)
              .on('change', on_flag_change_bool));
  } else if (t == 'string' || t == 'int' || t == 'long') {
    // td.append($("<p/>").text(f.getAttribute('value')))
    //   .append($("<input type='button' value='Edit'/>"));
    td.append($("<input type='text'/>")
              .attr('data-flag', f.name)
              .attr('value', f.value)
              .attr('original', f.value)
              .on('input change', on_flag_input_text)
              .css('width', '160px'));  // TODO
    td.append($("<div class='controls hidden'></div>")
              .append($("<img src='img/small-check.png'/>").on('click', on_flag_check))
              .append("&nbsp;")
              .append($("<img src='img/small-cross.png'/>").on('click', on_flag_cross)));
  } else {
    td.text(f.getAttribute('value'));
  }
  return td;
}

function handle_start_race_button() {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'timer.remote-start'}
         });
}

function show_mask() {
  var nlanes = $("#n-lanes").val();
  if (nlanes <= 0) {
    nlanes = 1;
  }
  var mask = $("#unused-lane-mask").val();
  for (var i = 0; i < nlanes; ++i) {
    var bit = 1 << i;
    // (mask & bit) is 1 if the lane is unused
    $("table#lanes td[data-bit=" + bit + "] img")
      .toggleClass('hidden', (mask & bit) != 0);
  }
}
$(function() { show_mask(); });

function on_mask_click(event) {
  var mask = $("#unused-lane-mask").val();
  var target = $(event.currentTarget);
  var bit = target.attr('data-bit');

  mask ^= bit;
  target.find('img').toggleClass('hidden', (mask & bit) != 0);

  $("#unused-lane-mask").val(mask);

  $.ajax("action.php",
         {type: 'POST',
          data: {action: 'timer.test',
                 'tt-mask': mask}
         });
}

function on_testing_change(event, synthetic) {
  if (!synthetic) {
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'timer.test',
                   'test-mode': $("#test-mode").is(':checked') ? 1 : 0,
                  },
           });
  }
}
$(function() { $("#test-mode").on('change', on_testing_change); });

function on_send_log_change(event) {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'settings.write',
                 'timer-send-logs': $("#timer-send-logs").is(':checked') ? 1 : 0,
                },
         });
}
$(function() { $("#timer-send-logs").on('change', on_send_log_change); });

function is_in_testing_mode(current) {
  if (! current) {
    return false;
  }
  return current.roundid == -100 &&
    current["now_racing"];
}
function update_testing_mode(current) {
  var should_be_checked = is_in_testing_mode(current);
  if ($("#test-mode").is(':checked') != should_be_checked) {
    $("#test-mode").prop('checked', should_be_checked);
    $("#test-mode").trigger('change', /*synthetic*/true);
  }
}

function update_timer_summary(tstate, details, current) {
  $("#timer_status_text").text(tstate.message);
  $("#timer_summary_icon").attr('src', tstate.icon);
  $("#start_race_button_div").toggleClass('hidden',
                                          !is_in_testing_mode(current) ||
                                          !tstate["remote-start"]);
  // Offer the fake timer only if no other timer is connected.
  $("#fake_timer_div").toggleClass('hidden', tstate.state != "1");
}

function update_timer_details(details) {
  $("#timer-details").empty();
  if (details.human) {
    $("<p></p>").text(details.human).appendTo($("#timer-details"));
  } else if (details.type) {
    $("<p></p>").text(details.type).appendTo($("#timer-details"));
  }
  if (details.ident) {
    $("<p></p>").text(details.ident).appendTo($("#timer-details"));
  }
  if (details.options) {
    $("<p></p>").text(details.options).appendTo($("#timer-details"));
  }
}

$(function() {
  var heat_showing = -1;
  var poll_interval = setInterval(function() {
    $.ajax('action.php',
           {type: 'GET',
            data: {query: 'poll.timer.test'},
            success: function(data) {
              if (data["cease"]) {
                clearInterval(poll_interval);
                window.location.href = '../index.php';
                return;
              }
              var tstate = data["timer-state"];
              var current = data["current-heat"];
              var details = data["timer-details"];
              update_timer_summary(tstate, details, current);
              update_timer_details(details);
              if (data.tt.heat != heat_showing) {
                $("table#lanes td.time, table#lanes td.place").text("");
                heat_showing = data.tt.heat;
              }
              update_testing_mode(current);
              $("#unused-lane-mask").val(data.tt.mask);
              show_mask();

              var tt = data.tt.results;
              for (var i = 0; i < tt.length; ++i) {
                var lane = tt[i].lane;
                if (tt[i].hasOwnProperty('time')) {
                  $("table#lanes tr.lane[data-lane=" + lane + "] td.time").text(tt[i].time);
                }
                if (tt[i].hasOwnProperty('place')) {
                  $("table#lanes tr.lane[data-lane=" + lane + "] td.place").text(tt[i].place);
                }
              }
            }
           });
  }, 500);
});


function poll_for_timer_log(seek, timeout) {
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'timer.log',
                 seek: seek},
          success: function(data) {
            if (data.hasOwnProperty('outcome')) {
              console.log(data);
            }
            if (data.hasOwnProperty('log-file-name')) {
              $("#log-location").text(data['log-file-name']);
            }
            if (data.hasOwnProperty('file-data')) {
              $("#log_text").append(document.createTextNode(data['file-data']));
              $("#log_container").scrollTop($("#log_container").scrollTop()
                                            + $("#log_text")[0].getBoundingClientRect().bottom
                                            - $("#log_container").height());
              timeout = 50;
            } else {
              timeout = 2 * timeout;
              if (timeout > 1000) {
                timeout = 1000;
              }
            }

            if (data.hasOwnProperty('file-size')) {
              seek = data['file-size'];
            } else {
              seek = 0;
            }
            setTimeout(function() { poll_for_timer_log(seek, timeout); }, timeout);
          }
         });
}

function on_resize() {
  $("#log_container").height($(window).height() -
                             $("#log_container")[0].getBoundingClientRect().top - 15);
}

$(function() {
    $("#log-setting-form input").on('change', function(e) {
        $.ajax("action.php",
               {type: 'POST',
                data: $("#log-setting-form").serialize()});
    });

  on_resize();
  $(window).resize(on_resize);

  poll_for_timer_log(0, 50);
  });
