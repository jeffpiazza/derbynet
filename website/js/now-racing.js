
// TODO: Incomplete race results are more likely than you'd think.
// TODO: Animation should only occur once, even if current heat doesn't advance.
// So: should detect when there's a complete result, animate it once, and then wait for a new heat.

// TODO: Window resizing should adjust the font size in some meaningful way.
// TODO: Test display in 640x480 with the longest names we can find.

// TODO: HttpTest stops on received failure, but doesn't resume...

var g_roundid = 1;
var g_heat = 1;

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

function animate_flyers(place, place_to_lane) {
    if (place >= place_to_lane.length) {
        $('.place').css({opacity: 100});
        $('.flying').animate({opacity: 0}, 1000);
    } else {
        console.log('typeof place_to_lane[' + place + '] = ' + typeof place_to_lane[place]);
        var flyer = $('#place' + place);
        var target = $('[data-lane="' + place_to_lane[place] + '"] .place');
        console.log('target.outerWidth=' + target.outerWidth() + ', target.outerHeight=' + target.outerHeight()
                    + ', target.width=' + target.width() + ', target.height=' + target.height()
                    + ', border=' + parseInt(target.css('border-top'))
                    + ', padding=' + parseInt(target.css('padding-top')));
        var border = parseInt(target.css('border-top'));
        var padding = parseInt(target.css('padding-top'));
        console.log('target vertical-align=' + target.css('vertical-align'));
        console.log('target line-height=' + target.css('line-height'));
        console.log('target has ' + target.children().length + ' child(ren)');
        console.log('target position: {top:' + target.position().top + ', left:' + target.position().left + '}');
        console.log('target offset: {top:' + target.offset().top + ', left:' + target.offset().left + '}');
        console.log('target font-size: ' + parseInt(target.css('font-size')));
        var span =  $('[data-lane="' + place_to_lane[place] + '"] .place span');
        console.log('target span height: ' + span.height());
        console.log('target span position top:' + span.position().top);
        console.log('target span line-height: ' + span.css('line-height'));
        console.log('target span height: ' + span.height() + ', outerHeight: ' + span.outerHeight());
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
                          animate_flyers(place + 1, place_to_lane);
                      });
    }
}

// See ajax/query.watching.inc for XML format

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
            $('[data-lane="' + lane + '"] .place').css({opacity: 0});
            $('[data-lane="' + lane + '"] .place span').text(place);
            if (hr.getAttribute('speed') != '') {
                $('[data-lane="' + lane + '"] .speed')
                    .css({opacity: 100})
                    .text(hr.getAttribute('speed'));
            }
        }

        animate_flyers(1, place_to_lane);
        setTimeout(function() {
            process_new_heat(watching);
        }, 4000);  // Wait 4 seconds after animation
    } else {
        process_new_heat(watching);
    }
}

function process_new_heat(watching) {
    var current = watching.getElementsByTagName("current-heat")[0];
    g_roundid = current.getAttribute("roundid");
    g_heat = current.getAttribute("heat");
    if (current.firstChild) {
        $('.banner_title').text(current.firstChild.data + ', Heat ' + g_heat);
    }

    var racers = watching.getElementsByTagName("racer");
    if (racers.length > 0) {
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
            // $('[data-lane="' + lane + '"] .carname').text(r.getAttribute('carname'));
            // $('[data-lane="' + lane + '"] .carnumber').text(r.getAttribute('carnumber'));
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
