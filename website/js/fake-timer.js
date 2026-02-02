'use strict';

// td[data-running] attribute has values:
//    0 masked off
//    1 running
//    2 stopped

// Most recently received xml <heat-ready> element
var g_pending_heat;
// setTimeout id for the heat_ready
var g_heat_ready_timer_id = 0;

// The setInterval identifier of the timer update function.
// 0 implies the "timer" is not running.
var g_timer_interval = 0;

var g_auto_mode = false;
// Seconds between heat ready and race start when in auto mode.
var g_auto_mode_pace = 5;
// The setTimeout id created by prepare_heat in auto mode -- start_timer will be called.
var g_prepared_heat = 0;


function on_lane_count_change() {
  var nlanes = $("#lane-count").val();
  $("#timer-sim th").remove();
  $("#timer-sim td").remove();
  for (var i = 0; i < nlanes; ++i) {
    $("#timer-sim-headers").append($("<th></th>").text("Lane " + (i + 1)));
    $("#timer-sim-times").append($("<td></td>").attr('data-running', 1));
    $("#timer-sim-stop-buttons").append(
      $("<td></td>")
        .append("<input type='button' value='Stop' onclick='stop_timer_now(this);'/>"));
  }
  g_pending_heat = 0;
  reset_timer();
}

$(function() {
  on_lane_count_change();
  $("#lane-count").on('keyup mouseup', on_lane_count_change);
});



function reset_timer() {
  $("#timer-sim-times td").attr('data-running', 1).text('0.000');
  $('#start-button').prop('disabled', true);
}

function prepare_heat(mask) {
  $("#timer-sim-times td").each(function(i, td) {
    if ((mask & (1 << i)) == 0) {
      $(td).attr('data-running', 0).text('---');
    } else {
      $(td).attr('data-running', 1).text('0.000');
    }
  });
  $('#start-button').prop('disabled', false);
  if (g_auto_mode) {
    console.log('g_prepared_heat setTimeout for start_timer in ' + g_auto_mode_pace);
    g_prepared_heat = setTimeout(function() { g_prepared_heat = 0; start_timer(); },
                                 g_auto_mode_pace * 1000);
  }
}

function stop_timer_now(button) {
  var index = $(button).closest('td').index();
  var td = $("#timer-sim-times td")[index];
  stop_timer(index, $(td).text());
}

function stop_timer(index, text) {
  var td = $("#timer-sim-times td")[index];
  td = $(td);
  if (td.attr('data-running') == 1 && g_timer_interval != 0) {
    td.text(text).attr('data-running', 2);
  }
  if ($("#timer-sim-times td[data-running='1']").length == 0) {
    end_race();
  }
}

function end_race() {
  if (g_timer_interval != 0) {
    clearInterval(g_timer_interval);
    g_timer_interval = 0;
    send_finished();
  }
}

function start_timer() {
  var start = (new Date()).getTime();
  g_timer_interval =
  setInterval(function() {
    var tr = $("#timer-sim-times");
    var t = (new Date()).getTime() - start;
    if (false && t > 5000) {
      end_race();
    }
    t = (t / 1000).toFixed(3);
    tr.find('td').each(function(i, td) {
      td = $(td);
      if (td.attr('data-running') == 1) {
        td.text(t);
      }
    });
  }, 1);

  send_started();
  $('#start-button').prop('disabled', true);
  if (g_auto_mode) {
    // Set lane stop events at random times
    $("#timer-sim-times td").each(function(i) {
      var when = (2.8 + Math.random()) * 1000;
      setTimeout(function(ii, ww) {
        return function() {
          stop_timer(ii, (ww / 1000).toFixed(3));
        }
      }(i, when), when);
    });
  }
}


$(function() { reset_timer(); });

function show_not_racing() {
  $('#summary').html("Not racing.");
}

function process_timer_messages(data) {
  data = $(data);

  // <heat-ready lane-mask= class= round= roundid= heat=/>
  // <abort/>

  var abort = data.find('abort');
  if (abort.length > 0) {
    console.log('abort heat');
    show_not_racing();
    reset_timer();
    $('#start-button').prop('disabled', true);
    g_pending_heat = 0;
    clearTimeout(g_heat_ready_timer_id);
    g_heat_ready_timer_id = 0;
    clearTimeout(g_prepared_heat);
    g_prepared_heat = 0;
  }

  var heat = data.find('heat-ready');
  if (heat.length > 0) {
    heat = $(heat[0]);
    $("#summary").html('Ready for heat <span class="heatno">' + heat.attr('heat') + '</span> of<br/>' +
                       '<span class="round-class">' + heat.attr('class') + '</span>, ' +
                       'round <span class="roundno">' + heat.attr('round') + '</span>');
    if (g_pending_heat &&
        g_pending_heat.attr('roundid') == heat.attr('roundid') &&
        g_pending_heat.attr('heat') == heat.attr('heat') &&
        g_pending_heat.attr('lane-mask') == heat.attr('lane-mask')) {
      // Do nothing
    } else {
      g_pending_heat = heat;
      if (g_heat_ready_timer_id == 0) {
        g_heat_ready_timer_id = setTimeout(function() {
          if (g_pending_heat) {
            prepare_heat(g_pending_heat.attr('lane-mask'));
          }
          g_heat_ready_timer_id = 0;
        }, 1000);
      }
    }
  }
}

function send_hello() {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'timer-message',
                 message: 'HELLO'},
          success: function(data) {
            if (typeof data == "object" && data["cease"]) {
              window.location.href = '../index.php';
              return;
            }
            process_timer_messages(data);
            send_identified();
          }
         });
}

function send_identified() {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'timer-message',
                 message: 'IDENTIFIED',
                 lane_count: $("#lane-count").val(),
                 timer: 'FakeTimer',
                 human: 'Fake Timer',
                 // ident: '(Ident TBD)',
                 // options: 'fake:true'
                },
          success: function(data) {
            if (typeof data == "object" && data["cease"]) {
              window.location.href = '../index.php';
              return;
            }
            process_timer_messages(data);
          }
         });
}

function send_heartbeat() {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'timer-message',
                 message: 'HEARTBEAT',
                 confirmed: 1},
          success: function(data) {
            if (typeof data == "object" && data["cease"]) {
              window.location.href = '../index.php';
              return;
            }
            process_timer_messages(data);
          }
         });
}


function send_started() {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'timer-message',
                 message: 'STARTED',
                 confirmed: 1},
          success: function(data) {
            if (typeof data == "object" && data["cease"]) {
              window.location.href = '../index.php';
              return;
            }
            process_timer_messages(data);
          }
         });
}


function send_finished() {
  var data = {action: 'timer-message',
              message: 'FINISHED'};
  $("#timer-sim-times td").each(function(i, td) {
    data['lane' + (i + 1)] = $(td).text();
  });

  $.ajax('action.php',
         {type: 'POST',
          data: data,
          success: function(data) {
            if (typeof data == "object" && data["cease"]) {
              window.location.href = '../index.php';
              return;
            }
            show_not_racing();  // Will be immediately changed back, most likely.
            process_timer_messages(data);
          }
         });
}

$(function() {
  send_hello();

  // Send a heartbeat every 1 second;
  setInterval(send_heartbeat, 1000);
});

function on_auto_mode_change() {
  g_auto_mode = !g_auto_mode;
  if (g_auto_mode && !$('#start-button').prop('disabled')) {
    start_timer();
  }
}

$(function() {
  $('#auto-mode-checkbox').on('change', on_auto_mode_change);
});

