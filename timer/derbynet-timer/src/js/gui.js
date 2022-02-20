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
    $("#racing-round").text(group + (round > 1 ? " round " + round : '') + ", heat " + heat);
    Gui._make_lanes("#next-lanes", lanes, mask);
    $("#next-lanes .rlane").each(function (i, lane) {
      $(lane).append("<img src='img/timer/down.png'/>");
    });
  }

  static mask_lanes(lanes, mask) {
    Gui._make_lanes("#racing-lanes", lanes, mask);
    $("#next-lanes .rlane").animate({opacity: 0});
  }

  // 1-based lane
  static lane_result(lane) {
    $("#racing-lanes .rlane").eq(lane - 1).empty().append("<img src='img/timer/checkerboard.png'/>");
  }

  static show_event(evt) {
    const MAX_EVENTS = 8;
    if ($("#events div").length > MAX_EVENTS - 1) {
      $("#events div").slice(0, $("#events div").length - MAX_EVENTS + 1).remove();
    }
    $("#events").append($("<div/>").text(evt));
  }

  static trouble_message(err) {
    console.error(err);
    $("#messages").prepend($("<p/>").text(err.toString())
                           .prepend("<img src='img/timer/trouble-tiny.png'/>"));
  }
}
