'use strict';

// This function receives messages from the surrounding replay kiosk, if there
// is one.
function on_message(msg) {
  if (msg == 'replay-started') {
    Poller.suspended = true;
  } else if (msg == 'replay-ended') {
    Poller.suspended = false;
  }
}
$(function() { window.onmessage = function(e) { on_message(e.data); }; });


var Poller = {
  // Timestamp when the last polling request was sent; used by watchdog to
  // detect a failure to queue a new request.
  time_of_last_request: 0,

  // Records the identifier of the timeout that will send the next polling
  // request, or 0 if no request is queued.
  id_of_timeout: 0,

  // If we're being shown within a replay iframe, suspend polling while a replay
  // is showing and we're not visible; resume when we have the display again.
  // We keep setting timeouts (which makes the watchdog happy), but when
  // suspended, poll_for_update just queues another request rather than asking
  // the server for anything.
  suspended: false,

  // Queues the next poll request when processing of the current request has
  // completed, including animations, if any.  Because animations are handled
  // asynchronously, with completion callbacks, we can't just start the next
  // request when process_now_racing_element returns.
  queue_next_request: function(roundid, heat) {
    if (this.id_of_timeout != 0) {
      console.log("Trying to queue a polling request when there's already one pending; ignored.");
    } else {
      Poller.id_of_timeout = setTimeout(function() {
        Poller.id_of_timeout = 0;
        Poller.poll_for_update(roundid, heat);
      }, 500);  // 0.5 sec
    }
  },

  poll_for_update: function(roundid, heat) {
    if (typeof(simulated_poll_for_update) == "function") {
      simulated_poll_for_update();
    } else if (this.suspended) {
      Poller.queue_next_request(roundid, heat);
    } else {
      this.time_of_last_request = Date.now();
      $.ajax('action.php',
             {type: 'GET',
              data: {query: 'poll',
                     values: 'best-times,current-heat,heat-results,precision,racers,' +
                             'timer-trouble,current-reschedule',
                     'head-size': g_column_width + 'x' + g_racer_photo_height,
                     'car-size': g_column_width + 'x' + g_car_photo_height,
                     roundid: roundid,
                     heat: heat},
              success: function(data) {
                process_polling_result(data);
              },
              error: function() {
                Poller.queue_next_request(roundid, heat);
              }
             });
    }
  },

  // Correct operation depends on queuing a new request when we're done
  // processing the last one, including any animations (could take a few
  // seconds).  As a safeguard against a bug that perhaps prevents queuing the
  // next request, the watchdog periodically tests whether one seems overdue,
  // and may queue a new request if so.
  watchdog: function() {
    if (this.id_of_timeout != 0 && this.time_of_last_request + 15000 < Date.now()) {
      console.error("Watchdog notices no requests lately, and none queued!");
      this.queue_next_request();
    }
  }
};

class ResultAnimator {
  // Set to true during animation and the "linger" time that follows
  animation_running;
  // Set to true to avoid repeating the animation
  results_written;

  constructor() {
    this.reset();
  }

  reset() {
    this.animation_running = false;
    this.results_written = false;
  }

  // Returns true if it's OK to update the page to show a new heat (i.e., the
  // results of the previous heat have lingered long enough for the viewers).
  ok_to_change() { return ! this.animation_running; }

  undoHeatResults() {
    $("div.lane div.heat_time").html("&nbsp;");
    $("div.lane div.place").text("").css({background: 'transparent'});
    this.reset();
  }

  async onHeatResults(precision, heat_results) {
    if (this.animation_running || this.results_written) {
      return;
    }
    this.animation_running = true;

    heat_results = heat_results.slice();
    heat_results.sort((hr1, hr2) => { return hr1.place - hr2.place; });

    for (var i = 0; i < heat_results.length; ++i) {
      let lane = heat_results[i].lane;
      let racer_entry_div = $("div.lane").eq(lane_to_column(lane)).find("div.racer-entry:first");
      let time_div = racer_entry_div.find("div.heat_time");
      time_div.text(Number.parseFloat(heat_results[i].time).toFixed(precision))
        .css({'background-color': 'red'});
      let time_animation = new Promise(function(result) {
        time_div.animate({'backgroundColor': '#c0c0c0'},
                         300,
                         function() { result(true); });
      });
      let place_div = racer_entry_div.find("div.place");
      // Instead of text, it might look better to work up images for the place,
      // with background shadow, etc., as needed.
      place_div.text(i + 1)
        .css({'background-color': 'white'});
      let place_animation = new Promise(function(result) {
        place_div.animate({'background-color': 'rgba(255,255,255,0.6)'},
                          300,
                          function() { result(true); });
      });
      await Promise.all([time_animation, place_animation]);
    }
    this.results_written = true;

    setTimeout(function(anim) {
      anim.animation_running = false;
    }, 10000, this);
  }
}

var g_result_animator = new ResultAnimator();

function vcenter_image(event) {
  var target = $(event.target);
  // "parent" is either div.car (which has a div.number to account for), or div.racer (which has div.name).
  // This doesn't account for div.carname or div.note, because we want the images to line up across racers,
  // and not bounce up and down depending on what attributes the racer has.
  var parent_height = target.parent().height() - target.parent().find('div.name').height()
      - target.parent().find('div.number').height();

  target.css('margin-top', (parent_height - target.height()) / 2);
  target.css('margin-left', (target.parent().width() - target.width()) / 2);
}

function update_best_times(best_times) {
  var tiles = $("div.bottom-banner div.best-time");
  for (var i = 0; i < best_times.length; ++i) {
    if (i < tiles.length) {
      tiles.eq(i).find('.carno').text(best_times[i].carnumber);
      tiles.eq(i).find('.time').text(best_times[i].finishtime);
    } else {
      $("div.bottom-banner").append(
        $('<div class="best-time"/>')
          .append($('<div class="carno"/>').text(best_times[i].carnumber))
          .append($('<div class="time"/>').text(best_times[i].finishtime)));
    }
  }
}


function process_polling_result(data) {
  var current_heat = data["current-heat"];

  if (data.hasOwnProperty('timer-trouble')) {
    Overlay.show('#timer_overlay');
  } else if (!current_heat["now_racing"] && g_result_animator.ok_to_change()) {
    Overlay.show('#paused_overlay');
  } else if (data["current-reschedule"]) {
    Overlay.show('#reschedule_overlay');
  } else {
    Overlay.clear();
  }

  var heat_results = data["heat-results"];
  if (heat_results && heat_results.length > 0) {
    g_result_animator.onHeatResults(data.precision, heat_results);
  } else {
    g_result_animator.undoHeatResults();
  }

  update_best_times(data['best-times']);

  var heat_key = current_heat.roundid + "+" + current_heat.heat;
  var now_showing = $("div.rollable").first().find("div.racer-entry").eq(0).attr("data-heat-key");
  if (heat_key != now_showing) {
    var next_up = $("div.rollable").first().find("div.racer-entry").eq(1).attr("data-heat-key");
    if (heat_key != next_up) {
      $("div.rollable").each(function(i, rollable) {
        $(rollable).find("div.racer-entry").eq(1).remove();
      });
      for (var lane = 1; lane <= g_number_of_lanes; ++lane) {
        var racer_entry =
            $("<div class='racer-entry'/>")
            .appendTo($("div.lane div.rollable").eq(lane_to_column(lane)))
            .attr('data-heat-key', heat_key)
            .attr('data-roundid', current_heat.roundid)
            .attr('data-heat', current_heat.heat)
            .append($("<div>&nbsp;</div>").addClass("heat_time"))
            .append($("<div/>").addClass("car")
                    .append($("<div/>").addClass("carname"))
                    .append($("<div/>").addClass("number"))
                    .append($("<div/>").addClass("place")))
            .append($("<div/>").addClass("racer")
                    .append($("<div/>").addClass("note"))
                    .append($("<div/>").addClass("name")));
      }

      for (var i = 0; i < data.racers.length; ++i) {
        var lane = data.racers[i].lane;
        var racerid = data.racers[i].racerid;
        var racer_entry = $("div.lane").eq(lane_to_column(lane)).find("div.racer-entry").last();
        racer_entry.attr('data-racerid', racerid);

        let div_car = racer_entry.find("div.car");
        if (data.racers[i].photo2) {
          div_car.prepend($("<img/>")
                          .attr('src', data.racers[i].photo2)
                          .css({'max-width': g_column_width_exact})
                          .on('load', vcenter_image));
        }
        div_car.find("div.number").text(data.racers[i].carnumber);
        if (data.racers[i].carname) {
          div_car.find("div.carname").css({'display': 'block'}).text(data.racers[i].carname);
        }

        let div_racer = racer_entry.find("div.racer");
        if (data.racers[i].photo) {
          div_racer.prepend($("<img/>")
                            .attr('src', data.racers[i].photo)
                            .css({'max-width': g_column_width_exact})
                            .on('load', vcenter_image));
        }
        div_racer.find("div.name").text(data.racers[i].name);
        if (data.racers[i].note) {
          div_racer.find("div.note").css({'display': 'block'}).text(data.racers[i].note);
        }
      }
    }

    now_showing = $("div.rollable").first().find("div.racer-entry").eq(0).attr("data-heat-key");
    if (heat_key != now_showing && g_result_animator.ok_to_change()) {
      // && $(':animated').length == 0 ?
      var title = '';
      if (current_heat['number-of-heats'] > 0 && current_heat.heat) {
        title = "Heat " + current_heat.heat + " of " + current_heat['number-of-heats'];
        if (current_heat.hasOwnProperty('class') && current_heat['class']) {
          title = current_heat['class'] + ', ' + title;
        }
      }
      $("div.banner_title").text(title);
      var height = $("div.rollable").height();
      $("div.rollable div.racer-entry:first-child").animate({'margin-top': -height}, 1000)
        .promise().then(function() {
              setTimeout(function() {
                $("div.rollable div.racer-entry:first-child")
                  .each(function(i, en) {
                    var p = $(en).parent();
                    $(en).css('margin-top', '')
                      .remove().appendTo(p);
                  });
                g_result_animator.reset();
                queue_next_request();
              }, 500);
            });
    } else {
      queue_next_request();
    }
  } else {
    queue_next_request();
  }
}

// It might be better to keep track of the display state explicitly, and move
// process_polling_result into its own class.
function queue_next_request() {
  var now_showing_div = $("div.rollable").first().find("div.racer-entry").eq(0);
  Poller.queue_next_request(now_showing_div.attr('data-roundid'), now_showing_div.attr('data-heat'));
}

$(window).on('resize', function() { location.reload(true); });

function next_heat() {
  $("div.rollable div.racer-entry:first-child").animate(
    {'margin-top': -$("div.rollable").height()}, 1000)
  .promise().then(function() {
      setTimeout(function() {
          $("div.rollable div.racer-entry:first-child")
          .each(function(i, en) {
              console.log(en);
              var p = $(en).parent();
              $(en).css('margin-top', '')
              .remove().appendTo(p);
              });
        }, 500);
    });
}

$(function() { $("div.lane").css("width", g_column_width_exact); });

$(function() {
    $('div.racer img').on('load', vcenter_image);
    $('div.car img').on('load', vcenter_image);
    $('div.bottom-banner').on('click', next_heat);
    $(window).on('click', next_heat);
  });


$(function () {
  Poller.poll_for_update(0, 0);
  // Run the watchdog every couple seconds
  setInterval(function() { Poller.watchdog(); }, 2000);
});
