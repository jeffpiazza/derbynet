
// This function receives messages from the surrounding replay kiosk, if there
// is one.
function on_message(msg) {
  if (msg == 'replay-started') {
    // TODO There's a race here: if the flyers have already started when replay takes
    // over the screen, the audience may not get to see the whole thing.
    Poller.suspended = true;
    Lineup.hold();
  } else if (msg == 'replay-ended') {
    // Start the timer that blocks advancing to the next heat
    Lineup.start_display_linger();
    Lineup.release();
    Poller.suspended = false;
    FlyerAnimation.enable_flyers();
  }
}
$(function() { window.onmessage = function(e) { on_message(e.data); }; });

var Lineup = {
  // heat and roundid identify the lineup currently displayed.  They're also
  // sent in the polling query to tell the server what lineup we'd want results
  // for.
  roundid: 0,
  heat: 0,
  // Normally we just acquire more heat results for the current heat, as they're
  // reported by the timer.  If the number of <heat-result> elements decreases
  // for a heat, we need to clear out the old values by treating the update like
  // it's a new heat.  previous_heat_results tracks how many <heat-result>
  // elements were in the previous update.
  previous_heat_results: 0,

  // While hidden by a replay, don't advance the display.
  holding: false,
  hold: function() {
    this.holding_display = true;
  },
  release: function() {
    this.holding = false;
  },

  // display_lingers_until tells when it's OK to advance the display to the next
  // heat.  Value is a timestamp in milliseconds.
  display_lingers_until: 0,

  // After an animation of heat results, hold the display for a few seconds
  // before advancing to the next heat.
  start_display_linger: function() {
    this.display_lingers_until = Date.now() + g_linger_ms;
  },

  ok_to_change: function() {
    return !this.holding && Date.now() > this.display_lingers_until;
  },

  // When the current heat differs from what we're presently displaying,
  // we get a <current-heat/> element, plus some number of <racer>
  // elements identifying the new heat's contestants.
  process_new_lineup: function(data, row_height) {
    var current = data["current-heat"];
    if (data.hasOwnProperty('timer-trouble')) {
      Overlay.show('#timer_overlay');
    } else if (!current["now_racing"] && this.ok_to_change()) {
      Overlay.show('#paused_overlay');
    } else if (data["current-reschedule"]) {
      Overlay.show('#reschedule_overlay');
    } else {
      Overlay.clear();
    }

    if (data.hasOwnProperty('timer-test')) {
      // In timer-test mode, we hide several columns (marked .no-test) and add
      // in their place a single-cell .test-only column showing the
      // "timer-testing" image.  When not in timer-test mode, undo all of that.
      if ($('td.test-only').length == 0) {
        $(".no-test").addClass('hidden');
        $("tr#table-headers th").first().after("<th class='test-only'/>");
        $("tr[data-lane=1] td.lane").after("<td class='test-only' rowspan='" + $("td.lane").length + "'>"
                                           + "<img src='img/timer-testing.png'/><br/>"
                                           + "Timer testing"
                                           + "</td>");
      }
    } else {
      if ($('td.test-only').length > 0) {
        $(".test-only").remove();
        $(".no-test").removeClass("hidden");
      }
    }
    // We always need to notice an increase in the number of heat-results, in
    // case they get cleared before the ok_to_change() test lets us update the
    // screen.
    var new_heat_results =
        data.hasOwnProperty('heat-results') ? data["heat-results"].length : 0;
    if (new_heat_results > this.previous_heat_results) {
      this.previous_heat_results = new_heat_results;
    }

    if (this.ok_to_change()) {
      var new_roundid = current.roundid;
      var new_heat = current.heat;
      var is_new_heat = new_roundid != this.roundid || new_heat != this.heat
          || new_heat_results < this.previous_heat_results;
      this.roundid = new_roundid;
      this.heat = new_heat;
      this.previous_heat_results = new_heat_results;

      var nheats = current['number-of-heats'];
      if (nheats) {
        var round_class_name = current['class'];
        if (current['use_master_sched']) {
          var title = 'Heat ' + current['masterheat'];
          if (round_class_name) {
            title += ': ' + round_class_name;
            if (current['round'] && current['round'] > 1) {
              title += ', round ' + current['round'];
            }
          }
          $('.banner_title').text(title);
        } else {
          $('.banner_title').text((round_class_name ? round_class_name + ', ' : '')
                                  + 'Heat ' + this.heat + ' of ' + nheats);
        }
      }

      var racers = data.racers;
      var zero = data.precision == 4 ? '0.0000' : '0.000';

      if (is_new_heat && racers.length > 0) {
        FlyerAnimation.enable_flyers();
        FlyerAnimation.set_number_of_racers(racers.length);

        FontAdjuster.reset();
        // Clear old results
        $('[data-lane] .carnumber').text('');
        $('[data-lane] .photo').empty();
        $('[data-lane] .name').text('');
        $('[data-lane] .time').css({opacity: 0}).text(zero);
        $('[data-lane] .speed').css({opacity: 0}).text('200.0');
        $('[data-lane] .place span').text('');
        $('[data-lane] .photo img').remove();
        for (var i = 0; i < racers.length; ++i) {
          var r = racers[i];
          var lane = r.lane;
          $('[data-lane="' + lane + '"] .lane').text(lane);
          $('[data-lane="' + lane + '"] .name').html('<div></div>');
          $('[data-lane="' + lane + '"] .name div').text(r.name);
          if (r.hasOwnProperty('photo') && r.photo != '') {
            $('[data-lane="' + lane + '"] .photo').html(
              $('<img/>')
                .attr('src', r.photo)
                .css('max-height', row_height)
            );
          }

          if (r.hasOwnProperty('carname') && r.carname != '') {
            $('[data-lane="' + lane + '"] .name').append($("<div class='carname'/>").text(r.carname));
          }
          if (r.hasOwnProperty('note') && r.note != '') {
            $('[data-lane="' + lane + '"] .name').append($("<div class='subtitle'/>").text(r.note));
          }

          $('[data-lane="' + lane + '"] .carnumber').text(r.carnumber);
        }
      } else if (racers.length > 0) {

        // Same heat, but possibly updated photo paths
        for (var i = 0; i < racers.length; ++i) {
          var r = racers[i];
          var lane = r.lane;
          if (r.hasOwnProperty('photo') && r.photo != '') {
            if ($('[data-lane="' + lane + '"] .photo img').length == 0) {
              // A window resize, below, may have removed the <img/> element on an interim basis
              $('[data-lane="' + lane + '"] .photo').html(
                $('<img/>')
                  .attr('src', r.photo)
                  .css('max-height', row_height)
              );
            }
            if (r.photo != $('[data-lane="' + lane + '"] .photo img').attr('src')) {
              $('[data-lane="' + lane + '"] .photo img').attr('src', r.photo);
            }
          } else {
            $('[data-lane="' + lane + '"] .photo').empty();
          }
        }
      }
    }

    // NOTE Any failure to get here will cause the page to get stuck.
    Poller.queue_next_request(this.roundid, this.heat);
  }
};


var FlyerAnimation = {
  ok_to_animate: true,

  enable_flyers: function() {
    this.ok_to_animate = true;
  },

  // Once the animation has started, avoid starting again until explicitly
  // re-enabled with a new heat or a request to repeat the animation.
  flyers_started: function() {
    this.ok_to_animate = false;
  },

  // If we get an incomplete update (that is, some but not all heat results from
  // the current heat were written to the database at the moment the <heat-result>
  // elements got generated), then we may be missing some of the possible places:
  // we might have 1st and 3rd place, but not 2nd and 4th.
  //
  // It's the holes in the place_to_lane mapping that screws us: we only attempt
  // to stop the animation when we go beyond place_to_lane.length; otherwise, we
  // assume we can find a place_to_lane entry for 2nd place if place_to_lane is
  // big enough to account for 3rd place.
  //
  // The test for a complete set of heat-results is that we have as many
  // heat-results as lanes.
  number_of_racers: 0,
  set_number_of_racers: function(n) {
    this.number_of_racers = n;
  },

  should_animate: function(nresults) {
    return this.ok_to_animate && nresults >= this.number_of_racers;
  },

  // Javascript passes arrays (like place_to_lane) by reference, so no
  // worries about doing a lot of copying in this recursion.
  //
  // This is implemented recursively because we use the completion
  // callback for animate to advance to the next flyer.
  //
  // The animate_flyers function doesn't really benefit from being in an
  // object -- it passes around all the state it needs.
  animate_flyers: function(place, place_to_lane, completed) {
    if (place >= place_to_lane.length) {
      $('.place').css({opacity: 100});
      $('.flying').animate({opacity: 0}, 1000);
      completed();
    } else {
      var flyer = $('#place' + place);
      var target = $('[data-lane="' + place_to_lane[place] + '"] .place');
      if (target.length > 0) {
        var border = parseInt(target.css('border-top-width'));
        // Apparently WebKit and Gecko (at least) model collapsed table
        // borders borders differently; in particular, target.offset() is
        // affected; we need to compensate by changing the border
        // adjustment.  Knowing that the CSS specifies 14px borders for this
        // table, we use the reported border width to distinguish between
        // the two engines.
        if (border < 14) {
          border = 0;
        }
        var span =  $('[data-lane="' + place_to_lane[place] + '"] .place span');
        var font_size = span.css('font-size');
        flyer.css({left: -target.outerWidth(),
                   width: target.outerWidth(),
                   top: target.offset().top - border,
                   height: target.outerHeight(),
                   fontSize: font_size, // as a string, e.g., 85px
                   padding: 0,
                   opacity: 100});
        flyer.animate({left: target.offset().left - border},
                      300,
                      function() {
                        FlyerAnimation.animate_flyers(place + 1, place_to_lane, completed);
                      });
      } else {
        console.log("Couldn't find an entry for " + place + "-th place.");
        FlyerAnimation.animate_flyers(place + 1, place_to_lane, completed);
      }
    }
  }
};

var g_row_height;
$(function() {
  Poller.build_request = function(roundid, heat) {
    // TODO It shouldn't be necessary to send row-height to the server;
    // instead just construct a racer photo URL from the returned racerid.
    g_row_height = 0;
    var photo_cells = $('td.photo');
    var border = parseInt(photo_cells.css('border-bottom-width'));

    if (photo_cells.length > 0) {
      // Position of the first td.photo may get adjusted
      g_row_height =
        Math.floor(($(window).height() - photo_cells.position().top) / photo_cells.length) - border;
    }

    return {query: 'poll',
            values: 'current-heat,heat-results,precision,racers,' +
            'timer-trouble,current-reschedule',
            roundid: roundid,
            heat: heat,
            'row-height': g_row_height };
  };
});

// Walks through each of the heat-result {lane= time= place= speed=} elements,
// in order, building a mapping from the reported place to the matching lane.
//
function process_polling_result(data) {
  var precision = data.hasOwnProperty('precision') ? data.precision : 3;
  var heat_results = data["heat-results"];
  if (heat_results && heat_results.length > 0) {
    var place_to_lane = new Array();  // place => lane
    for (var i = 0; i < heat_results.length; ++i) {
      var hr = heat_results[i];
      var lane = hr.lane;
      var place = hr.place;
      place_to_lane[parseInt(place)] = lane;

      $('[data-lane="' + lane + '"] .time')
        .css({opacity: 100})
        .text(Number.parseFloat(hr.time).toFixed(precision));
      if (FlyerAnimation.ok_to_animate) {
        $('[data-lane="' + lane + '"] .place').css({opacity: 0});
      }
      $('[data-lane="' + lane + '"] .place span').text(place);
      if (hr.speed != '') {
        $('[data-lane="' + lane + '"] .speed')
          .css({opacity: 100})
          .text(hr.speed);
      }
    }

    if (FlyerAnimation.should_animate(heat_results.length)) {
      FlyerAnimation.flyers_started();
      FlyerAnimation.animate_flyers(1, place_to_lane, function () {
        // Need to continue to poll for repeat-animation, just not
        // accept new participants for 10 seconds.
        Lineup.start_display_linger();
        Lineup.process_new_lineup(data, g_row_height);
      });
    } else {
      Lineup.process_new_lineup(data, g_row_height);
    }
  } else {
    Lineup.process_new_lineup(data, g_row_height);
  }
}

// Update the table height to fill the window below the title bar, then adjust
// the margin on the "flyer" elements.  The flyer height and width will be set
// when the animation actually runs.
function resize_table() {
  // Since images have a fixed size, they can cause the table to be too tall for
  // the new window size.  We temporarily remove the photos and rely on
  // process_new_heat, above, to repopulate with different-sized photos.
  $("table td.photo").empty();
  $("table").css({height: $(window).height() - 60});

  var place = $('[data-lane="1"] .place');
  var btop = parseInt(place.css('border-top'));
  var mtop = parseInt(place.css('margin-top'));
  $('.flying').css({margin: mtop + btop});

  FontAdjuster.table_resized();
}

$(function () {
  resize_table();
  $(window).resize(function() { resize_table(); });

  // This 1-second delay is to let the initial resizing take effect
  setTimeout(function() { Poller.poll_for_update(0, 0); }, 1000);
  // Run the watchdog every couple seconds
  setInterval(function() { Poller.watchdog(); }, 2000);
});
