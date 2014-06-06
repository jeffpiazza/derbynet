
// TODO: Incomplete race results are more likely than you'd think.
// TODO: Animation should only occur once, even if current heat doesn't advance.
// So: should detect when there's a complete result, animate it once, and then wait for a new heat.

// TODO: CSS beautification
// TODO: Window resizing should adjust the font size in some meaningful way.

// TODO: HttpTest stops on received failure, but doesn't resume...
// TODO: Rename this kiosk and associated files before check-in!
// TODO: Who's supposed to format the times?

var g_roundid = 1;
var g_heat = 1;

function watching_fire() {
  var xmlhttp = new XMLHttpRequest();
  xmlhttp.open("GET", "action.php?query=watching&roundid=" + g_roundid + "&heat=" + g_heat, /*async*/true);
  xmlhttp.onreadystatechange = watching_handler;
  xmlhttp.send("");
}

function watching_handler() {
  if (this.readyState == this.DONE) {
	if (this.status == 200) {
	  if (this.responseXML != null) {
	    $('#ajax_failure').addClass('hidden');
	    process_watching(this.responseXML.documentElement);
	  } else {
        // If the text returned from update-summary isn't parsable, e.g.,
        // because there's some kind of error on the php side, then
        // responseXML can come back null.  Rather than completely
        // freak out, let's try again in a moment.
		console.log("XmlHttpResponse:");
		console.log(this);
	    $('#ajax_status').html("Response from server doesn't parse as XML.");
	    $('#ajax_failure').removeClass('hidden');
        setTimeout(watching_fire, 200);  // 0.2 sec
      }
	} else {
	  $('#ajax_status').html(this.status + " (" + 
							 (this.status == 0 ? "likely timeout" : this.statusText)
							 + ")");
	  $('#ajax_failure').removeClass('hidden');
      setTimeout(watching_fire, 200);  // 0.2 sec
 	}
  }
}

function animate_flyers(place, flyers) {
    if (place >= flyers.length) {
        $('.place').css({opacity: 100});
        $('.flying').animate({opacity: 0}, 1000);
    } else {
        var flyer = $('#place' + place);
        var target = $('[data-lane="' + flyers[place] + '"] .place');
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
        var span =  $('[data-lane="' + flyers[place] + '"] .place span');
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
                          animate_flyers(place + 1, flyers);
                      });
    }
}

//   <watching roundid= heat= >
//     <heat-result lane="1" time="" place="" speed=""/>... if results are available
//     <current-heat> 
//     -- if current-heat differs from what the caller passed in, then provide
//     --    <racer lane="1" name="Jimmy Jones" carname="Greased Lightning" carnumber="" photo=""/>
//   </watching>

function process_watching(watching) {
    var heat_results = watching.getElementsByTagName("heat-result");
    if (heat_results.length > 0) {
        var flyers = new Array();  // place => lane
        for (var i = 0; i < heat_results.length; ++i) {
            var hr = heat_results[i];
            var lane = hr.getAttribute('lane');
            var place = hr.getAttribute('place');
            flyers[parseInt(place)] = lane;

            $('[data-lane="' + lane + '"] .time').text(hr.getAttribute('time').substring(0,5));
            $('[data-lane="' + lane + '"] .place').css({opacity: 0});
            $('[data-lane="' + lane + '"] .place span').text(place);
            $('[data-lane="' + lane + '"] .speed').text(hr.getAttribute('speed'));
        }

        animate_flyers(1, flyers);
        setTimeout(function() {
            process_new_heat(watching);
        }, 4000);
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
        $('[data-lane] td .name').text('');
        $('[data-lane] td .time').text('');
        $('[data-lane] td .speed').text('');
        for (var i = 0; i < racers.length; ++i) {
            var r = racers[i];
            var lane = r.getAttribute('lane');
            $('[data-lane="' + lane + '"] .lane').text(lane);
            $('[data-lane="' + lane + '"] .name').text(r.getAttribute('name'));
            // $('[data-lane="' + lane + '"] .carname').text(r.getAttribute('carname'));
            // $('[data-lane="' + lane + '"] .carnumber').text(r.getAttribute('carnumber'));
        }
    }

    setTimeout(watching_fire, 200);  // 0.2 sec
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
    watching_fire();
});
