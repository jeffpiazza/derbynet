var Overlay = {
  overlay_shown: '',

  clear: function() {
    $(".overlay_foreground").fadeTo(200, 0);
    $("#overlay_background").fadeTo(200, 0);
    this.overlay_shown = '';
  },

  show: function(selector) {
    if (this.overlay_shown != selector) {
      var background = $("#overlay_background");
      background.css({'display': 'block',
                      'opacity': 0});
      background.fadeTo(200, 0.5);
      $(".overlay_foreground").css('opacity', 0);

      var overlay = $(selector);
      overlay.css({
        'display': 'block',
        'position': 'fixed',
        'opacity': 0,
        'z-index': 11000,
        'left' : '50%',
        'margin-left': '-256px',
        'top': $(window).height() / 2 + "px",
        'margin-top': '-256px'
      });
      overlay.fadeTo(200, 1);
      this.overlay_shown = selector;
    }
  }
};

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

  // hold_display_until tells when it's OK to change the display.  Value is a
  // timestamp in milliseconds.
  hold_display_until: 0,

  // After an animation of heat results, hold the display for a few seconds
  // before advancing to the next heat.
  hold_display: function() {
    this.hold_display_until = (new Date()).valueOf() + 10000;
  },

  ok_to_change: function() {
    return (new Date()).valueOf() > this.hold_display_until;
  },

  // When the current heat differs from what we're presently displaying,
  // we get a <current-heat/> element, plus some number of <racer>
  // elements identifying the new heat's contestants.
  process_new_lineup: function(now_racing) {
    if (now_racing.getElementsByTagName("hold-current-screen").length > 0) {
      // Each time a hold-current-screen element is sent, reset the
      // hold-display deadline.  (Our display is presumed not to be visible,
      // so the display-for-ten-seconds clock shouldn't start yet.)
      this.hold_display();
    }
    var current = now_racing.getElementsByTagName("current-heat")[0];
    if (now_racing.getElementsByTagName('timer-trouble').length > 0) {
      Overlay.show('#timer_overlay');
    } else if (current.getAttribute("now-racing") == "0" && this.ok_to_change()) {
      Overlay.show('#paused_overlay');
    } else {
      Overlay.clear();
    }

    // We always need to notice an increase in the number of heat-results, in
    // case they get cleared before the ok_to_change() test lets us update the
    // screen.
    var new_heat_results = now_racing.getElementsByTagName("heat-result").length;
    if (new_heat_results > this.previous_heat_results) {
      this.previous_heat_results = new_heat_results;
    }

    if (this.ok_to_change()) {
      var new_roundid = current.getAttribute("roundid");
      var new_heat = current.getAttribute("heat");
      var is_new_heat = new_roundid != this.roundid || new_heat != this.heat
          || new_heat_results < this.previous_heat_results;
      this.roundid = new_roundid;
      this.heat = new_heat;
      this.previous_heat_results = new_heat_results;

      if (current.firstChild) {  // The body of the <current-heat>
        // element names the class
        $('.banner_title').text(current.firstChild.data
                                + ', Heat ' + this.heat
                                + ' of ' + current.getAttribute('number-of-heats'));
      }

      var racers = now_racing.getElementsByTagName("racer");
      if (is_new_heat && racers.length > 0) {
        FlyerAnimation.enable_flyers();
        FlyerAnimation.set_number_of_racers(racers.length);

        FontAdjuster.reset();
        // Clear old results
        $('[data-lane] .carnumber').text('');
        $('[data-lane] .photo').empty();
        $('[data-lane] .name').text('');
        $('[data-lane] .time').css({opacity: 0}).text('0.000');
        $('[data-lane] .speed').css({opacity: 0}).text('200.0');
        $('[data-lane] .place span').text('');
        $('[data-lane] img').remove();
        for (var i = 0; i < racers.length; ++i) {
          var r = racers[i];
          var lane = r.getAttribute('lane');
          $('[data-lane="' + lane + '"] .lane').text(lane);
          $('[data-lane="' + lane + '"] .name').html('<div></div>');
          $('[data-lane="' + lane + '"] .name div').text(r.getAttribute('name'));
          if (r.hasAttribute('photo') && r.getAttribute('photo') != '') {
            $('[data-lane="' + lane + '"] .photo').html(
              '<img src="' + r.getAttribute('photo') + '"/>');
          }

          if (r.hasAttribute('carname') && r.getAttribute('carname') != '') {
            $('[data-lane="' + lane + '"] .name').append(' <span id="carname-' + lane + '" class="subtitle"/>');
            $('#carname-' + lane).text('"' + r.getAttribute('carname') + '"');
          }
          if (r.hasAttribute('subgroup')) {
            $('[data-lane="' + lane + '"] .name').append(' <span id="subgroup-' + lane + '" class="subtitle"/>');
            $('#subgroup-' + lane).text(r.getAttribute('subgroup'));
          }

          $('[data-lane="' + lane + '"] .carnumber').text(r.getAttribute('carnumber'));
        }
      } else if (racers.length > 0) {
        // Same heat, but possibly updated photo paths
        for (var i = 0; i < racers.length; ++i) {
          var r = racers[i];
          var lane = r.getAttribute('lane');
          if (r.hasAttribute('photo') && r.getAttribute('photo') != '') {
            if ($('[data-lane="' + lane + '"] .photo img').length == 0) {
              // A window resize, below, may have removed the <img/> element on an interim basis
              $('[data-lane="' + lane + '"] .photo').html(
                '<img src="' + r.getAttribute('photo') + '"/>');
            }
            if (r.getAttribute('photo') != $('[data-lane="' + lane + '"] .photo img').attr('src')) {
              $('[data-lane="' + lane + '"] .photo img').attr('src', r.getAttribute('photo'));
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

var Poller = {
  // Timestamp when the last polling request was sent; used by watchdog to
  // detect a failure to queue a new request.
  time_of_last_request: 0,

  // Records the identifier of the timer to send the next polling request, or 0
  // if no request is queued.
  id_of_timeout: 0,

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
    } else {
      var row_height = 0;
      var photo_cells = $('td.photo');
      var border = parseInt(photo_cells.css('border-bottom-width'));

      if (photo_cells.length > 0) {
        // Position of the first td.photo may get adjusted
        row_height = Math.floor(($(window).height() - photo_cells.position().top) / photo_cells.length) - border;
      }

      this.time_of_last_request = (new Date()).valueOf();
      $.ajax('action.php',
             {type: 'GET',
              data: {query: 'poll.now-racing',
                     roundid: roundid,
                     heat: heat,
                     'row-height': row_height},
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
    if (this.id_of_timeout != 0 && this.time_of_last_request + 15000 < (new Date()).valueOf()) {
      console.error("Watchdog notices no requests lately, and none queued!");
      this.queue_next_request();
    }
  }
};

// See ajax/query.poll.now-racing.inc for XML format

// Processes the top-level <now-racing> element.
//
// Walks through each of the <heat-result lane= time= place= speed=> elements,
// in order, building a mapping from the reported place to the matching lane.
//

function process_polling_result(now_racing) {
  var heat_results = now_racing.getElementsByTagName("heat-result");
  if (heat_results.length > 0) {
    // The presence of a <repeat-animation/> element is a request to re-run
    // the finish place animation, which we do by clearing the flag that
    // remembers we've already done it once.
    if (now_racing.getElementsByTagName("repeat-animation").length > 0) {
      FlyerAnimation.enable_flyers();
    }
    var place_to_lane = new Array();  // place => lane
    for (var i = 0; i < heat_results.length; ++i) {
      var hr = heat_results[i];
      var lane = hr.getAttribute('lane');
      var place = hr.getAttribute('place');
      place_to_lane[parseInt(place)] = lane;

      $('[data-lane="' + lane + '"] .time')
        .css({opacity: 100})
        .text(hr.getAttribute('time'));
      if (FlyerAnimation.ok_to_animate) {
        $('[data-lane="' + lane + '"] .place').css({opacity: 0});
      }
      $('[data-lane="' + lane + '"] .place span').text(place);
      if (hr.getAttribute('speed') != '') {
        $('[data-lane="' + lane + '"] .speed')
          .css({opacity: 100})
          .text(hr.getAttribute('speed'));
      }
    }

    if (FlyerAnimation.should_animate(heat_results.length)) {
      FlyerAnimation.flyers_started();
      FlyerAnimation.animate_flyers(1, place_to_lane, function () {
        // Need to continue to poll for repeat-animation, just not
        // accept new participants for 10 seconds.
        Lineup.hold_display();
        Lineup.process_new_lineup(now_racing);
      });
    } else {
      Lineup.process_new_lineup(now_racing);
    }
  } else {
    Lineup.process_new_lineup(now_racing);
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
