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

function handle_timer_settings_button() {
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'timer.settings'},
          success: function(data) {
            open_timer_settings_modal(data);
          }});
}

function open_timer_settings_modal(data) {
  var ports = data.documentElement.getElementsByTagName('ports');
  if (ports.length > 0) {
    ports = ports[0].getAttribute('value').split(',');
  } else {
    ports = [];
  }
  $("#timer_settings_port select").empty();
  $("#timer_settings_port select").append("<option value='' selected='selected'>Auto Port</option>");
  for (var i = 0; i < ports.length; ++i) {
      $("<option/>")
        .attr('value', ports[i])
        .text(ports[i])
        .appendTo($("#timer_settings_port select"));
    }

  var devices = data.documentElement.getElementsByTagName('device');
  $("#timer_settings_device select").empty();
  $("#timer_settings_device select").append("<option value='' selected='selected'>Auto Device</option>");
  for (var i = 0; i < devices.length; ++i) {
    $("<option/>")
      .attr('value', devices[i].getAttribute('name'))
      .text(devices[i].textContent)
      .appendTo($("#timer_settings_device select"));
  }
  
  var flags = data.documentElement.getElementsByTagName('flag');
  $("#timer_settings_modal_flags").empty();
  for (var i = 0; i < flags.length; ++i) {
    var f = flags[i];
    $("#timer_settings_modal_flags").append(
      $("<tr/>")
        .append($("<td/>").text(f.getAttribute('name')))
        .append($("<td/>").text(f.textContent))
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


function make_flag_control(f, td) {
  if (f.getAttribute('type') == 'bool') {
    td.append($('<input type="checkbox" class="flipswitch"'
                + (f.getAttribute('value') != 'false' ? ' checked="checked"' : '')
                + '/>')
              .attr('data-flag', f.getAttribute('name'))
              .on('change', on_flag_change_bool));
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
  return current.getAttribute('roundid') == -100 &&
    current.getAttribute('now-racing') == 1;
}
function update_testing_mode(current) {
  var should_be_checked = is_in_testing_mode(current);
  if ($("#test-mode").is(':checked') != should_be_checked) {
    $("#test-mode").prop('checked', should_be_checked);
    $("#test-mode").trigger('change', /*synthetic*/true);
  }
}

function update_timer_summary(tstate, current) {
  $("#timer_status_text").text(tstate.textContent);
  $("#timer_summary_icon").attr('src', tstate.getAttribute("icon"));
  $("#start_race_button_div").toggleClass('hidden',
                                          !is_in_testing_mode(current) ||
                                          tstate.getAttribute("remote_start") != "1");
}

function update_timer_details(details) {
  $("#timer-details").empty();
  if (details.getAttribute('human')) {
    $("<p></p>").text(details.getAttribute('human')).appendTo($("#timer-details"));
  } else if (details.getAttribute('type')) {
    $("<p></p>").text(details.getAttribute('type')).appendTo($("#timer-details"));
  }
  if (details.getAttribute('ident')) {
    $("<p></p>").text(details.getAttribute('ident')).appendTo($("#timer-details"));
  }
  if (details.getAttribute('options')) {
    $("<p></p>").text(details.getAttribute('options')).appendTo($("#timer-details"));
  }
}

$(function() {
  var heat_showing = -1;
  setInterval(function() {
    $.ajax('action.php',
           {type: 'GET',
            data: {query: 'poll.timer.test'},
            success: function(data) {
              var tstate = data.documentElement.getElementsByTagName('timer-state');
              var current = data.documentElement.getElementsByTagName('current-heat');
              if (tstate.length > 0) {
                update_timer_summary(tstate[0], current.length > 0 ? current[0] : undefined);
              }
              var details = data.documentElement.getElementsByTagName('timer-details');
              if (details.length > 0) {
                update_timer_details(details[0]);
              }
              var tt_results = data.documentElement.getElementsByTagName('tt-results');
              if (tt_results.length > 0 && tt_results[0].getAttribute('heat') != heat_showing) {
                $("table#lanes td.time, table#lanes td.place").text("");
                heat_showing = tt_results[0].getAttribute('heat')
              }
              if (current.length > 0) {
                update_testing_mode(current[0]);
              }
              var tt_mask = data.documentElement.getElementsByTagName('tt-mask');
              if (tt_mask.length > 0) {
                $("#unused-lane-mask").val(tt_mask[0].textContent);
                show_mask();
              }
              var tt = data.documentElement.getElementsByTagName('tt');
              for (var i = 0; i < tt.length; ++i) {
                var lane = tt[i].getAttribute('lane');
                var time = tt[i].getAttribute('time');
                if (time) {
                  $("table#lanes tr.lane[data-lane=" + lane + "] td.time").text(time);
                }
                var place = tt[i].getAttribute('place');
                if (place) {
                  $("table#lanes tr.lane[data-lane=" + lane + "] td.place").text(place);
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
             var file_data = data.documentElement.getElementsByTagName('file-data');
             if (file_data && file_data.length > 0) {
               file_data = file_data[0];
               $("#log_text").append(
                 document.createTextNode(file_data.textContent));
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

             var file_size_elt  = data.documentElement.getElementsByTagName('file-size');
             if (file_size_elt && file_size_elt.length > 0) {
               seek = file_size_elt[0].getAttribute('size');
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
