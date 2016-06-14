// TODO: Window resizing should adjust the font size in some meaningful way.
// TODO: Test display in 640x480 with the longest names we can find.
var g_roundid = 0;
var g_heat = 0;

// Compare number of racers we're talking about with number of
// heat-results received to determine if we got a complete update.
// Incomplete race results (which occur when the entries for some but
// not all lanes in the heat are ready when we poll for results)
// happen more frequently than you might think.
var g_num_racers = 0;

// Tells whether we've run the place animation for the current heat.
var g_animated = true;
// To prevent updating the display for a period of time after the race
// completes, g_hold_display_until gives the time in milliseconds after which
// it's OK to update.
var g_hold_display_until = 0;

// After an animation of heat results, hold the display this many milliseconds
// before advancing to the next heat.
var g_display_duration_after_animation = 10000;

// Queues the next poll request when processing of the current request has
// completed, including animations, if any.  Because animations are handled
// asynchronously, with completion callbacks, we can't just start the next
// request when process_watching returns.
function queue_next_poll_request() {
    setTimeout(poll_for_update, 500);  // 0.5 sec
}

function poll_for_update() {
    if (typeof(simulated_poll_for_update) == "function") {
        simulated_poll_for_update();
    } else {
        $.ajax('action.php',
               {type: 'GET',
                data: {query: 'watching',
                       roundid: g_roundid,
                       heat: g_heat},
                success: function(data) {
                    process_watching(data);
                },
                error: function() {
                    queue_next_poll_request();
                }
               });
    }
}

// Javascript passes arrays (like place_to_lane) by reference, so no
// worries about doing a lot of copying in this recursion.
//
// This is implemented recursively because we use the completion
// callback for animate to advance to the next flyer.

function animate_flyers(place, place_to_lane, completed) {
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
                              animate_flyers(place + 1, place_to_lane, completed);
                          });
        } else {
            console.log("Couldn't find an entry for " + place + "-th place.");
            animate_flyers(place + 1, place_to_lane, completed);
        }
    }
}

var g_overlay_shown = '';

function clear_overlay() {
    $(".overlay_foreground").fadeTo(200, 0);
    $("#overlay_background").fadeTo(200, 0);
    g_overlay_shown = '';
}

function show_overlay(selector) {
    if (g_overlay_shown != selector) {
        console.log(selector);
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
        g_overlay_shown = selector;
    }
}

// See ajax/query.watching.inc for XML format

// Processes the top-level <watching> element.
//
// Walks through each of the <heat-result lane= time= place= speed=> elements,
// in order, building a mapping from the reported place to the matching lane.
//
// If we get an incomplete update (that is, some but not all heat results from
// the current heat were written to the database at the moment the <heat-result>
// elements got generated), then we be missing some of the possible places: we
// might have 1st and 3rd place, but not 2nd and 4th.
//
// It's the holes in the place_to_lane mapping that screws us: we only attempt
// to stop the animation when we go beyond place_to_lane.length; otherwise, we
// assume we can find a place_to_lane entry for 2nd place if place_to_lane is
// big enough to account for 3rd place.
//
// The test for a complete set of heat-results is that we have as many
// heat-results as lanes.

function process_watching(watching) {
    var heat_results = watching.getElementsByTagName("heat-result");
    if (heat_results.length > 0) {
        // The presence of a <repeat-animation/> element is a request to re-run
        // the finish place animation, which we do by clearing the flag that
        // remembers we've already done it once.
        if (watching.getElementsByTagName("repeat-animation").length > 0) {
            g_animated = false;
        }
        var place_to_lane = new Array();  // place => lane
        for (var i = 0; i < heat_results.length; ++i) {
            var hr = heat_results[i];
            var lane = hr.getAttribute('lane');
            var place = hr.getAttribute('place');
            place_to_lane[parseInt(place)] = lane;

            $('[data-lane="' + lane + '"] .time')
                .css({opacity: 100})
                .text(hr.getAttribute('time').substring(0,5));
            if (!g_animated) {
                $('[data-lane="' + lane + '"] .place').css({opacity: 0});
            }
            $('[data-lane="' + lane + '"] .place span').text(place);
            if (hr.getAttribute('speed') != '') {
                $('[data-lane="' + lane + '"] .speed')
                    .css({opacity: 100})
                    .text(hr.getAttribute('speed'));
            }
        }

        // The g_num_racers test is to overcome the fact that sometimes it can
        // happen that some but not all of the results for a heat have been
        // recorded, in which case our determination of place order is
        // incomplete at best and likely wrong.
        if (!g_animated && heat_results.length >= g_num_racers) {
            g_animated = true;
            animate_flyers(1, place_to_lane, function () {
                // Need to continue to poll for repeat-animation, just not
                // accept new participants for 10 seconds.
                g_hold_display_until = (new Date()).valueOf() + g_display_duration_after_animation;
                queue_next_poll_request();
            });
        } else {
            process_new_heat(watching);
        }
    } else {
        process_new_heat(watching);
    }
}

// When the current heat differs from what we're presently displaying,
// we get a <current-heat/> element, plus some number of <racer>
// elements identifying the new heat's contestants.

function process_new_heat(watching) {
    if (watching.getElementsByTagName("hold-current-screen").length > 0) {
        // Each time a hold-current-screen element is sent, reset the
        // hold-display deadline.  (Our display is presumed not to be visible,
        // so the display-for-ten-seconds clock shouldn't start yet.)
        g_hold_display_until = (new Date()).valueOf() + g_display_duration_after_animation;
    }
    var current = watching.getElementsByTagName("current-heat")[0];
    if (watching.getElementsByTagName('timer-trouble').length > 0) {
        show_overlay('#timer_overlay');
    } else if (current.getAttribute("now-racing") == "0" && (new Date()).valueOf() > g_hold_display_until) {
        show_overlay('#paused_overlay')
    } else {
        clear_overlay();
    }

    if (current.getAttribute("now-racing") != "0" && (new Date()).valueOf() > g_hold_display_until) {
        g_roundid = current.getAttribute("roundid");
        g_heat = current.getAttribute("heat");
        if (current.firstChild) {  // The body of the <current-heat>
                                   // element names the class
            $('.banner_title').text(current.firstChild.data 
                                    + ', Heat ' + g_heat
                                    + ' of ' + current.getAttribute('number-of-heats'));
        }

        var racers = watching.getElementsByTagName("racer");
        if (racers.length > 0) {
            g_animated = false;
            g_num_racers = racers.length;
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
                $('[data-lane="' + lane + '"] .name').text(r.getAttribute('name'));
                if (r.hasAttribute('photo') && r.getAttribute('photo') != '') {
                  $('[data-lane="' + lane + '"] .photo').prepend(
                      '<img src="' + r.getAttribute('photo') + '"/>');
                }
                var br_added = false;
                if (r.hasAttribute('carname') && r.getAttribute('carname') != '') {
                    if (!br_added) {
                        $('[data-lane="' + lane + '"] .name').append('<br/>');
                        br_added = true;
                    }
                    $('[data-lane="' + lane + '"] .name').append(' <span id="carname-' + lane + '" class="subtitle"/>');
                    $('#carname-' + lane).text('"' + r.getAttribute('carname') + '"');
                }
                if (r.hasAttribute('subgroup')) {
                    if (!br_added) {
                        $('[data-lane="' + lane + '"] .name').append('<br/>');
                        br_added = true;
                    }
                    $('[data-lane="' + lane + '"] .name').append(' <span id="subgroup-' + lane + '" class="subtitle"/>');
                    $('#subgroup-' + lane).text(r.getAttribute('subgroup'));
                }

                $('[data-lane="' + lane + '"] .carnumber').text(r.getAttribute('carnumber'));
            }
        }
    }

    queue_next_poll_request();
}

// Update the table height to fill the window below the title bar, then adjust
// the margin on the "flyer" elements.  The flyer height and width will be set
// when the animation actually runs.
function resize_table() {
    $("table").css({height: $(window).height() - 60});

    var place = $('[data-lane="1"] .place');
    var btop = parseInt(place.css('border-top'));
    var mtop = parseInt(place.css('margin-top'));
    $('.flying').css({margin: mtop + btop});
}

$(function () {
    resize_table();
    $(window).resize(function() { resize_table(); });
    poll_for_update();
});
