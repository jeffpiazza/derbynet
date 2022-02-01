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

  static prepare_heat(group, round, heat, lanes, mask) {
    $("#racing-round").text(group + (round > 1 ? " round " + round : '') + ", heat " + heat);
    if ($("#racing-lanes .rlane").length != lanes) {
      $("#racing-lanes").empty();
      for (var lane = 1; lane <= lanes; ++lane) {
        $("#racing-lanes").append($("<span class='rlane'></span>").text(lane));
      }
      // 4 is the border size for each lane box
      $("#racing-lanes .rlane").css(
        'width', ($("#racing-lanes").width() / lanes - 4) + 'px');
    }

    $("#racing-lanes .rlane").removeClass('masked');
    for (var zlane = 0; zlane < lanes; ++zlane) {
      if ((mask & (1 << zlane)) == 0) {
        console.log('Masking lane ' + (zlane + 1));
        $("#racing-lanes .rlane").eq(zlane).addClass('masked');
      }
    }
  }

  static show_event(evt) {
    const MAX_EVENTS = 8;
    if ($("#events div").length > MAX_EVENTS - 1) {
      $("#events div").slice(0, $("#events div").length - MAX_EVENTS + 1).remove();
    }
    $("#events").append($("<div/>").text(evt));
  }
}
