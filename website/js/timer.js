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

function update_testing_mode(current) {
  var should_be_checked =
      current.getAttribute('roundid') == -100 && current.getAttribute('now-racing') == 1;
  if ($("#test-mode").is(':checked') != should_be_checked) {
    $("#test-mode").prop('checked', should_be_checked);
    $("#test-mode").trigger('change', /*synthetic*/true);
  }
}

function update_timer_summary(tstate) {
  $("#timer_status_text").text(tstate.textContent);
  $("#timer_summary_icon").attr('src', tstate.getAttribute("icon"));
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
              if (tstate.length > 0) {
                update_timer_summary(tstate[0]);
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
              var current = data.documentElement.getElementsByTagName('current-heat');
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
