'use strict';


var Poller = {
  // Timestamp when the last polling request was sent; used by watchdog to
  // detect a failure to queue a new request.
  time_of_last_request: 0,

  // Records the identifier of the timeout that will send the next polling
  // request, or 0 if no request is queued.
  id_of_timeout: 0,

  // If we're being shown within a replay iframe, suspend polling while a replay
  // is showing and we're not visible; resume when we have the display again.
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
                     values: 'best-times,current-heat,heat-results,precision,racers,timer-trouble',
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
  // Set to true during animation and "linger" time that follows
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

  waiting_for_results() { return ! this.results_written; }
  ok_to_change() { return ! this.animation_running; }

  async onHeatResults(precision, heat_results) {
    if (this.animation_running || this.results_written) {
      return;
    }
    this.animation_running = true;

    heat_results = heat_results.slice();
    heat_results.sort((hr1, hr2) => { return hr1.place - hr2.place; });

    for (var i = 0; i < heat_results.length; ++i) {
      let lane = heat_results[i].lane;
      let div = $("div.lane").eq(lane - 1).find("div.racer-entry:first div.heat_time");
      div.text(Number.parseFloat(heat_results[i].time).toFixed(precision))
        .css({'background-color': 'red'});
      await new Promise(function(result) {
        // setTimeout(function() {
        div.animate({'backgroundColor': '#c0c0c0'},
                    300,
                    function() { result(true); });
        // }, 1000);
      });
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
  var parent_height = target.parent().height() - target.parent().find('div.name').height();
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
  } else {
    Overlay.clear();
  }

  var heat_results = data["heat-results"];
  if (heat_results && heat_results.length > 0) {
    g_result_animator.onHeatResults(data.precision, heat_results);
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
            .appendTo($("div.lane div.rollable").eq(lane - 1))
            .attr('data-heat-key', heat_key)
            .attr('data-roundid', current_heat.roundid)
            .attr('data-heat', current_heat.heat)
            .append($("<div>&nbsp;</div>").addClass("heat_time"))
            .append($("<div/>").addClass("car")
                    .append($("<div/>").addClass("name")))
            .append($("<div/>").addClass("racer")
                    .append($("<div/>").addClass("name")));
      }

      var lane_width = $(window).width() / g_number_of_lanes;
      var car_wxh = lane_width + 'x' +
          ($("div.lane div.car").height() - $("div.lane div.car div.name").height());
      var racer_wxh = lane_width + 'x' +
          ($("div.lane div.racer").height() - $("div.lane div.racer div.name").height());

      // TODO Cache breaker for racer photos?
      for (var i = 0; i < data.racers.length; ++i) {
        var lane = data.racers[i].lane;
        var racerid = data.racers[i].racerid;
        var racer_entry = $("div.lane").eq(lane - 1).find("div.racer-entry").last();
        racer_entry.attr('data-racerid', racerid);
        racer_entry.find("div.car")
          .prepend($("<img/>")
                   .attr('src', "photo.php/car/racer/" + racerid + "/" + car_wxh + "/0")
                   .css({'max-width': lane_width})
                   .on('load', vcenter_image))
          .find("div.name").text((data.racers[i].carnumber + ' ' + data.racers[i].carname).trim());
        racer_entry.find("div.racer")
          .prepend($("<img/>")
                   .attr('src', "photo.php/head/racer/" + racerid + "/" + racer_wxh + "/0")
                   .css({'max-width': lane_width})
                   .on('load', vcenter_image));
        racer_entry.find("div.racer").find("div.name").text(data.racers[i].name);
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

function resize_window() {
  var width = $(window).width();
  $("div.lane").css("width", width / g_number_of_lanes + "px");

  $("div.lane").each(function(i, div_lane) {
    $(div_lane).find("div.racer-entry").each(function(ii, div_racer_entry) {
      // TODO Bye lanes?  
      var racerid = $(div_racer_entry).attr('data-racerid');
      var racer = $(div_lane).find("div.racer");
      var wxh = racer.width() + 'x' + (racer.height() - racer.find('div.name').height());
      racer.find('img').remove();
      racer.prepend($("<img/>")
                    .attr('src', "photo.php/head/racer/" + racerid + "/" + wxh + "/0")
                    .css({'max-width': racer.width()})
                    .on('load', vcenter_image));

      var car = $(div_lane).find("div.car");
      wxh = car.width() + 'x' + (car.height() - car.find('div.name').height());
      car.find('img').remove();
      car.prepend($("<img/>")
                  .attr('src', "photo.php/car/racer/" + racerid + "/" + wxh + "/0")
                  .css({'max-width': racer.width()})
                  .on('load', vcenter_image));
    });
  });
}

$(window).on('resize', resize_window);

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

$(function() {
    var width = $(window).width();
    $("div.lane").css("width", width/g_number_of_lanes);
    
    resize_window();
});

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
