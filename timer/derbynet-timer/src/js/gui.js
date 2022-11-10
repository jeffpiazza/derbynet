'use strict';

class Gui {
  static probe_port(porti) {      
    $("#ports-list li").removeClass('probing chosen');
    if (porti >= 0) {
      $("#ports-list li").eq(porti).addClass('probing');
    }
  }
  static probe_port_trouble(porti) {
    $("#ports-list li").eq(porti).removeClass('probing').addClass('trouble');
  }

  static probe_profile(profi) {
    $("#profiles-list li").removeClass('probing');
    if (profi >= 0) {
      $("#profiles-list li").eq(profi).addClass('probing');
    }
  }

  static prober_complete(porti, profi) {
    $("#ports-list li").eq(porti)
      .removeClass('probing user-chosen').addClass('chosen');
    $("#profiles-list li").eq(profi)
      .removeClass('probing user-chosen').addClass('chosen');

    $("#racing-div").css('display', 'none')
      .removeClass('hidden')
      .slideDown(500);
    $("#profiles-div").addClass('racing', 500, function() {
      $("#profiles-div").scrollTop($("#profiles-list li").eq(profi).offset().top -
                                   $("#profiles-list li")          .offset().top);
    });
  }

  static prober_incomplete() {
    $("#ports-list li").removeClass('probing chosen user-chosen');
    $("#profiles-list li").removeClass('probing chosen user-chosen');
    $("#racing-div").slideUp(500);
    $("#profiles-div").scrollTop(0);
    $("#profiles-div").removeClass('racing', 500);
  }

  static onEvent(event, args) {
    switch (event) {
    case 'IDENTIFIED':
      Gui.set_serial_status(/*probed/confirmed?*/args[1] ? 'ok' : 'unknown');
      break;
    case 'PREPARE_HEAT_RECEIVED':
      // Gui.show_event(event);
      Gui.prepare_heat(/*group*/args[5], /*round*/args[4], /*heat*/args[1],
          /*lanes*/args[3], /*lanemask*/args[2]);
      break;
    case 'RACE_FINISHED':
      // Gui.show_event(event + " id=" + args[0] + " heat=" + args[1] + " " + args[2]);
      // Move current heat information to last-report-... divs
      console.log('RACE_FINISHED seen in gui');  // TODO
      $("#last-reported-round").text('Last result: ' + $("#racing-round").text());
      $("#last-reported-lanes").empty();
      $("#racing-lanes span.rlane").appendTo("#last-reported-lanes");
      break;
    case 'GATE_OPEN':
    case 'GATE_CLOSED':
      // Assumes that 'GATE_OPEN' or 'GATE_CLOSED' could only have come from the timer, and
      // therefore confirms that the timer is communicating.
      Gui.set_serial_status('ok');
      break;
    case 'MASK_LANES':
      // Gui.show_event(event + " " + (args || []).join(','));
      break;
    case 'LANE_RESULT': {
      Gui.set_serial_status('ok');
      // Gui.show_event(event + " " + (args || []).join(','));
      Gui.lane_result(/*lane*/args[0], /*time*/args[1]);
      break;
    }
    case 'LOST_CONNECTION':
      Gui.set_serial_status('not_connected');
      Gui.prober_incomplete();  // Back to the drawing board
      break;
    default:
      // Gui.show_event(event + " " + (args || []).join(','));
    }
  }

  static set_racing_state(state, is_knowable) {
    // state: IDLE, MARK, SET, RUNNING
  }

  // Populates span.rlane elements into a div;
  // masked lanes (per mask argument) get span.rlane.masked
  static _make_lanes(div, lanes, mask) {
    div = $(div);

    div.empty();
    for (var lane = 1; lane <= lanes; ++lane) {
      div.append($("<span class='rlane'></span>").text(lane));
    }

    var rlanes = div.find(".rlane");
    // 4 is the border size for each lane box
    rlanes.css('width', (div.width() / lanes - 4) + 'px');
    for (var zlane = 0; zlane < lanes; ++zlane) {
      if ((mask & (1 << zlane)) == 0) {
        rlanes.eq(zlane).addClass('masked');
      }
    }
  }

  static prepare_heat(group, round, heat, lanes, mask) {
    // XML heat-ready in server response generates PREPARE_HEAT, but then
    // timer_proxy triggers a MASK_LANES.
    $("#racing-round").text(group + (round > 1 ? " round " + round : '') + ", heat " + heat);
    Gui._make_lanes("#racing-lanes", lanes, mask);
  }

  // 1-based lane
  static lane_result(lane, time) {
    $("#racing-lanes .rlane").eq(lane - 1).empty().text(time);
  }

  static show_event(evt) {
    const MAX_EVENTS = 8;
    if ($("#events div").length > MAX_EVENTS - 1) {
      $("#events div").slice(0, $("#events div").length - MAX_EVENTS + 1).remove();
    }
    $("#events").append($("<div/>").text(evt));
  }

  static trouble_message(err) {
    $("#messages").prepend($("<p/>").text(err.toString())
                           .prepend("<img src='img/timer/trouble-tiny.png'/>"));
  }

  // png is the base name (without extension) from img/status, i.e., one of
  // 'ok', 'not_connected', 'trouble', or 'unknown'
  static set_serial_status(png) {
    $("#serial-status").prop('src', 'img/status/' + png + '.png');
  }
}

TimerEvent.register(Gui);

