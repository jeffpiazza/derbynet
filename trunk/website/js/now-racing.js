
// TODO: Incomplete race results are more likely than you'd think.
// TODO: Animation should only occur once, even if current heat doesn't advance.
// So: should detect when there's a complete result, animate it once, and then wait for a new heat.

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

function poll_for_update() {
    $.ajax('action.php',
           {type: 'GET',
            data: {query: 'watching',
                   roundid: g_roundid,
                   heat: g_heat},
            success: function(data) {
                process_watching(data);
            },
            error: function() {
                setTimeout(poll_for_update, 200);
            }
           });
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
            var border = parseInt(target.css('border-top'));
            var padding = parseInt(target.css('padding-top'));
            var span =  $('[data-lane="' + place_to_lane[place] + '"] .place span');
            var font_size = span.css('font-size');
            flyer.css({left: -target.outerWidth(),
                       width: target.outerWidth(),
                       top: target.offset().top - border,
                       height: target.outerHeight(),

                       fontSize: font_size, // as a string, e.g., 85px
                       // verticalAlign: 'middle',
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
                setTimeout(function() {
                    process_new_heat(watching);
                }, 10000);  // Wait 10 seconds after animation
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
    var current = watching.getElementsByTagName("current-heat")[0];
    if (current.getAttribute("now-racing") != "0") {
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
            $('[data-lane] .name').text('');
            $('[data-lane] .time').css({opacity: 0}).text('0.000');
            $('[data-lane] .speed').css({opacity: 0}).text('200.0');
            $('[data-lane] .place span').text('');
            for (var i = 0; i < racers.length; ++i) {
                var r = racers[i];
                var lane = r.getAttribute('lane');
                $('[data-lane="' + lane + '"] .lane').text(lane);
                $('[data-lane="' + lane + '"] .name').text(r.getAttribute('name'));
                if (r.hasAttribute('photo') && r.getAttribute('photo') != '') {
                    $('[data-lane="' + lane + '"] .name_and_photo').prepend(
                        '<img src="' + r.getAttribute('photo') + '"/>');
                } else {
                    $('[data-lane="' + lane + '"] img').remove();
                }
                // $('[data-lane="' + lane + '"] .carname').text(r.getAttribute('carname'));
                // $('[data-lane="' + lane + '"] .carnumber').text(r.getAttribute('carnumber'));
            }
        }
    }

    // Queue the next 
    setTimeout(poll_for_update, 200);  // 0.2 sec
}

function resize_table() {
    console.log("Resizing: window " + $(window).width() + 'w x ' + $(window).height());
    $("table").css({height: $(window).height() - 60});

    var place = $('[data-lane="1"] .place');
    var btop = parseInt(place.css('border-top'));
    var mtop = parseInt(place.css('margin-top'));
    var ptop = parseInt(place.css('padding-top'));
    console.log('padding-top=' + ptop + ', border-top=' + btop + ', margin-top=' + mtop);

    var box = Math.min(place.height(), place.width());
    console.log('height=' + place.height() + ' width=' + place.width() + ' box-size=' + box);

    //place.css({fontSize: box - btop - mtop});
    console.log('fontSize=' + (box - btop - mtop));
    $('.flying').css({//fontSize: box - btop - mtop,
        // verticalAlign: 'middle',
                      margin: mtop + btop,
        // padding: ptop
    });
    console.log('After, flying fontSize=' + $('.flying').css('font-size'));
}

$(function () {
    resize_table();
    $(window).resize(function() { resize_table(); });
    poll_for_update();
});
