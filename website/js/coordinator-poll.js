// Requires dashboard-ajax.js

// Data structure describing each round:
//
// round = { roundid:
//           class:
//           name:
//           round: (number)
//           roster_size, passed, unscheduled
//           heats_scheduled, heats_run
//           category: one of ("now-racing", "master-schedule",
//                             "ready-to-race", "not-yet-scheduled",
//                             "done-racing",)
//         }
// (racers_scheduled = passed - unscheduled)
// (master-schedule is applied to a synthesized 'totals' round.)
// (now-racing category is applied to the currently-running round.)
//
// A layout sorts the roundids into bins that determine the order in which they
// should be shown.
//
// layout = {'now-racing': []
//           'ready-to-race': [],
//           'not-yet-scheduled': []
//           'done-racing': [],
//          };
//
// g_rounds_layout records the last layout used, to determine whether the page
// really needs updating.

// Page structure is:
//
// div.double_control_column
//   div.scheduling_control_group      #now-racing-group
// div.control_column_container
//   div.control_column
//     div.control_group.heat_control_group
//       (Controls for racing, including #is-current-racing flipswitch.)
//     div.control_group               #supplemental-control-group
//       div                           #add-new-rounds-button
//       div                           #now-racing-group-buttons
//     div.control_group.timer_control_group
//       #timer_status_icon, other stuff
//     div.control_group.queue_control_group
//     div.control_group.replay_control_group
//       #replay_status_icon, replay controls
//   div.control_column
//     div.master_schedule_group       #master-schedule-group
//     div.scheduling_control_group    #ready-to-race-group
//     div.scheduling_control_group    #not-yet-scheduled-group
//     div.scheduling_control_group    #done-racing-group

// g_current_heat_racers remembers who the racers in the current heat
// are, so the modal for manual heat results can be populated when
// displayed.
g_current_heat_racers = new Array();

// The "current" heat as shown to the user.  The timer may cause the server's
// idea of the "current" heat to advance, but, like the now-racing page, we hold
// our "current" heat a few seconds beyond the reporting of results.
g_current_heat = {roundid: 0, heat: 0, hold_until: Date.now() - 1};

// To avoid rewriting (as opposed to updating) round controls constantly,
// keep this signature that gives the roundids in each category, in order.
// We only need to rewrite the round controls when this signature changes.
g_rounds_layout = {'now-racing': ["force a first run"],
                   'ready-to-race': [],
                   'not-yet-scheduled': [],
                   'done-racing': [],
                  };

// Processing a response from coordinator_poll can force the isracing
// flipswitch to change state.  We don't want that change to trigger
// an ajax request to update the database: if we get a little bit out
// of sync with the database, it'll put us into a feedback cycle that
// never ends.
//
// Use this boolean to mark changes induced programmatically, as
// opposed to user input.
g_updating_current_round = false;

// If any of the "new round" modals is open, we don't want to be
// rewriting all the controls.  This boolean tells whether we have the
// modal open.
g_new_round_modal_open = false;

// Each time a polling result arrives, we update this array describing the
// rounds that have completed.  If the user clicks on the "Add New Round"
// button, this array is used to populate both the choose_new_round_modal and
// the new_round_modal dialogs.
// Each entry is:
//    {classid, class, round, roundid, name, aggregate, category, subgroups,
//     heats_run,heats_scheduled,passed,racers_scheduled,racers_unscheduled,roster_size}
g_completed_rounds = [];

// Roundids of an aggregate rounds
g_aggregate_rounds = [];

function find_by_classid(classes, classid) {
  for (var i = 0; i < classes.length; ++i) {
    if (classes[i].classid == classid) {
      return classes[i];
    }
  }
}

function update_for_last_heat(json) {
  var rerun_type = json['last-heat'];
  var button = $("#rerun-button");
  var enable = true;
  button.attr("data-rerun", rerun_type);
  if (rerun_type == 'recoverable') {
    button.val("Reinstate Heat");
  } else {
    var results = false;
    for (var i = 0; i < json.racers.length; ++i) {
      if (json.racers[i]['finishtime'] || json.racers[i]['finishplace'] > 0) {
        results = true;
      }
    }
    if (results) {
      button.val("Re-Run This Heat");
      button.attr("data-rerun", 'current');
    } else if (rerun_type == 'none') {
      button.val("Re-Run");
      enable = false;
    } else /* rerun_type == 'available' */ {
      button.val("Re-Run Previous");
    }
  }
  button.prop('disabled', !enable);
}


// Generate page contents in response to poll.coordinator output

function update_for_current_round(current) {
    var isracing_checkbox = $("#is-currently-racing");
    if (isracing_checkbox.prop('checked') != current['now_racing']) {
        isracing_checkbox.prop('checked', current['now_racing']);
        g_updating_current_round = true;
        try {
            isracing_checkbox.trigger("change", true);
        } finally {
            g_updating_current_round = false;
        }
    }
}

function generate_timer_state_group(tstate) {
    $("#timer_status_text").text(tstate.message);
    $("#timer_status_icon").attr('src', tstate.icon);
    if (tstate.lanes != '' && tstate.lanes > 0) {
        $("#lane_count").text(tstate.lanes);
    }
}

function generate_replay_state_group(replay_state) {
  $("#replay_status").text(replay_state.message);
  $("#replay_status_icon").attr('src', replay_state.icon);
  $("#test_replay").toggleClass("hidden", !replay_state.connected);
}

// Updates progress bars with new progress values
function inject_progress_bars(control_group, round) {
    if (round.roster_size > 0) {
        // bar2 (yellow) = passed
        // bar1 (blue) = scheduled
        // Trouble here will be scheduled-but-no-longer-passed, i.e., scheduled > passed
      var passed = round.passed;
        control_group.find(".racers .bar2").width((100 * passed / round.roster_size) + '%');
        if (passed > 0) {
          control_group.find(".racers .bar1").width((100 * (round.passed - round.unscheduled) / passed) + '%');
        }
    }

    if (round.heats_scheduled > 0) {
        control_group.find(".heats .bar1").width((100 * round.heats_run / round.heats_scheduled) + '%');
    }
}

// Injects new progress values into the progress text
function inject_progress_text(control_group, round) {
    control_group.find("[data-name=roster_size]").text(round.roster_size);
    control_group.find("[data-name=n_passed]").text(round.passed);
    control_group.find("[data-name=scheduled]").text(round.passed - round.unscheduled);
    control_group.find("[data-name=n_heats_scheduled]").text(round.heats_scheduled);
    control_group.find("[data-name=n_heats_run]").text(round.heats_run);
}

// Constructs a control group div for this round:
//   div.heat-lineup, only for the current heat
//   div.roundno
//   h3
//   (progress text), collapsible
//   (progress bars)
//   div.collapsible[data-name='buttons']
//
// round = { roundid:
//           class:
//           round: (number)
//           roster_size, passed, racers_scheduled, heats_scheduled, heats_run
//           category: ("master-schedule", "ready-to-race", "not-yet-scheduled", "done-racing")
//         }
// current = { roundid: }
// timer_state = { lanes: }
//





//  div.control_group data-roundid=
//    h3.roundclass
//      img(triangle)
//    div.collapsible
//      p (racers, heats)
//    div.racers.progress
//      div.bar1(no)
//      div.bar2
//    div.block_buttons.collapsible

function generate_scheduling_control_group(round, current, timer_state) {
  var show_checkins = round.round == 1 && round.heats_scheduled == 0;
  
  var control_group = $("<div class=\"control_group scheduling_control\"></div>")
      .attr('data-roundid', round.roundid)
      .appendTo("#" + round.category + "-group")
      .append($("<h3 class=\"roundclass\"></h3>")
              .text(round.name)
              .prepend("<img data-name=\"triangle\" src=\"img/triangle_east.png\"/>"))
      .append($('<div class="collapsible"/>')
              .append('<p>'
                      + (show_checkins
                         ? '<span data-name="roster_size"></span> racer(s), '
                         + '<span data-name="n_passed"></span> passed, ' 
                         + '<span data-name="scheduled"></span> in schedule.'
                         + '<br/>'
                         : '')
                      + '<span data-name="n_heats_scheduled"></span> heats scheduled, '
                      + '<span data-name="n_heats_run"></span> run.'
                      + '</p>'))
      .append('<div class="racers heats progress">'
              + (show_checkins
                 ? "<div class='bar2'>"
                 + "<div class='bar1'></div>"
                 + "</div>"
                 + "</div>"
                 : "")
              + (round.heats_scheduled != 0
                 ? "<div class='bar1'></div>"
                 : "")
              + '</div>')
      .append($("<div data-name=\"buttons\" class=\"block_buttons collapsible\"/>"));

  if (round['next-round']) {
    control_group.find('h3.roundclass').append('<div class="next_tag">NEXT</div>');
  }

  control_group.find(".collapsible").hide();
  control_group.on('click', function() {
    var closed = control_group.find(".collapsible").css("display") == "none";
    // Don't want the current round to collapse like this...

    $("img[data-name=triangle]").attr('src', 'img/triangle_east.png');
    $(".collapsible").slideUp(200);

    if (closed) {
      control_group.find("img[data-name=triangle]").attr('src', 'img/triangle_south.png');
      control_group.find(".collapsible").slideDown(200); 
    }
  });

  inject_into_scheduling_control_group(round, current, timer_state);
}

// This is the double-wide section describing the current round, at the top of the page
function generate_current_round_control_group(round, current, timer_state) {
  var control_group = $("<div class=\"control_group scheduling_control\"></div>")
      .attr('data-roundid', round.roundid)
      .appendTo("#now-racing-group")
      .append($("<h3 class=\"roundclass\"></h3>")
              .text(round.name))
      .append($("<div class='heat-text'/>")
              .append($('<p>'
                        + '<span data-name="n_heats_scheduled"></span> heats scheduled, '
                        + '<span data-name="n_heats_run"></span> run.'
                        + '</p>'))
              .append('<div class="heat_number"/>'))
    .addClass('current')
    .append("<div class='heat-lineup'></div>");

  inject_into_scheduling_control_group(round, current, timer_state);
}

function reason_for_schedule_change(round) {
  var why = $('<ul/>');
  for (var a = 0; a < round.adjustments.length; ++a) {
    var who = round.adjustments[a];
    var li = $('<li/>').appendTo(why)
        .append($('<span/>').text(who.carnumber))
        .append(' ')
        .append($('<span/>').text(who.firstname + ' ' + who.lastname))
    if (who.why == 'unscheduled') {
      li.append(' isn\'t in the schedule.');
    } else {
      li.append(' is in the schedule but not the racing group.');
    }
  }
  return why;
}

// Injects new values into an existing scheduling control group.  The
// available control buttons get rewritten entirely.
function inject_into_scheduling_control_group(round, current, timer_state) {
  var control_group = $("[data-roundid=" + round.roundid + "]");

  inject_progress_text(control_group, round);
  inject_progress_bars(control_group, round);

  var buttons = control_group.find("[data-name=buttons]");
  if (round.roundid == current.roundid && round.roundid > 0) {
    buttons = $("#now-racing-group-buttons");
  }
  buttons.empty();
  buttons.parent().removeClass('adjustment-needed');

  if (round.roundid == -1) {
    buttons.append('<input type="button"'
                   + ' onclick="handle_master_next_up()" value="Next Up"/>');
  } else {
    if (round.heats_scheduled > 0 && round.heats_run == 0) {
      if (round.adjustments && round.adjustments.length > 0) {
        buttons
          .append($('<div class="late-arrival-prompt"></div>')
                  .append('<p>The race schedule needs to be regenerated because:</p>')
                  .append(reason_for_schedule_change(round))
                  .append('<p>Start by removing the existing race schedule.</p>'))
        .parent().addClass('adjustment-needed');
      }
      buttons.append('<input type="button"'
                     + ' onclick="handle_unschedule_button(' + round.roundid
                     + ', \'' + round['class'].replace(/"/g, '&quot;').replace(/'/, "\\'") + '\', '
                     + round.round + ')"'
                     + ' value="Unschedule"/>');
    }
    if (round.heats_scheduled == 0 && round.unscheduled > 0) {
      if (timer_state.lanes != '' && timer_state.lanes > 0) {
        buttons.append('<input type="button"'
                       + ' onclick="show_schedule_modal(' + round.roundid + ')"'
                       + ' value="Schedule"/>');
      } else {
        buttons.append("<p>Can't schedule heats, because the number of lanes hasn\'t" +
                       " been determined.<br/>" +
                       "Enter the number of lanes on the " +
                       "<a href='settings.php'>Settings</a> page.</p>");
      }
    }
    if (round.adjustments && round.adjustments.length > 0) {
      buttons
        .append($('<div class="late-arrival-prompt"></div>')
                .append('<p>The race schedule needs to be adjusted because:</p>')
                .append(reason_for_schedule_change(round)))
        .append('<input type="button" class="late-arrival-button"'
                + ' onclick="handle_reschedule_button(' + round.roundid + ')"'
                + ' value="Adjust Schedule"/>')
        .parent().addClass('adjustment-needed');
    }

    if (round.heats_scheduled == 0 && round.heats_run == 0 &&
        (round.round > 1 || round.aggregate)) {
      buttons.append('<input type="button"'
                     + ' onclick="handle_delete_round_button(' + round.roundid
                     + ', \'' + round['class'].replace(/"/g, '&quot;').replace(/'/, "\\'") + '\', '
                     + round.round + ')"'
                     + ' value="Delete Round"/>');
    }

    if (round.roundid != current.roundid) {
      // TODO: Don't offer 'race' choice for single roundid under master scheduling
      if (round.heats_run > 0) {
        buttons.append('<input type="button"'
                       + ' onclick="handle_make_changes_button(' + round.roundid + ')"'
                       + ' value="Make Changes"/>');
      } else if (round.heats_scheduled > 0 && round.heats_run < round.heats_scheduled) {
        buttons.append('<input type="button"'
                       + ' onclick="handle_race_button(' + round.roundid + ')"'
                       + ' value="Race"/>');
      }
    }
    if (round.heats_run > 0) {
        buttons.append('<input type="button"'
                       + ' onclick="handle_purge_button(' + round.roundid + ', ' + round.heats_run + ')"'
                       + ' value="Repeat Round"/>');
    }

    // TODO: AND there isn't already a next round or aggregate round with
    // finishtimes...
    // TODO: Un-generate a round?  GPRM allows deleting rounds, but
    // apparently not the first round.
    if (false && round.heats_scheduled > 0 && round.heats_run == round.heats_scheduled) {
      buttons.append('<input type="button"'
                     + ' onclick="show_new_round_modal(' + round.roundid + ')"'
                     + ' value="New Round"/>');
    }
  }
}

function generate_current_heat_racers(new_racers, current, nlanes) {
  var racers = g_current_heat_racers;
  // g_current_heat is the heat shown in the current heat field.
  // If that differs from the actual current round, then stick with the g_current_heat for the next
  // several seconds.
  if ((g_current_heat.roundid != current.roundid || g_current_heat.heat != current.heat) &&
      g_current_heat.hold_until == 0 && current.heat_results.length > 0) {
    console.log('Transitioning from ' + g_current_heat.roundid + ':' + g_current_heat.heat
                + ' to ' + current.roundid + ':' + current.heat);
    g_current_heat.hold_until = Date.now() + 10 * 1000;  // 10 seconds
  }

  if (racers.length == 0 && new_racers.length == 0) {
    return;
  }
  var holding = Date.now() < g_current_heat.hold_until;

  $("#now-racing-group .heat_number").empty()
    .append("<h3>Heat " + current.heat + " of "
            + current.heats_scheduled + "</h3>");
  var heat_lineup =
      $("#now-racing-group .heat-lineup").empty()
      .append($("<div class='racing'/>")
              .append(
                "<table>"
                  + "<tr>"
                  + "<th>Lane</th>"
                  + "<th>Car</th>"
                  + "<th>Racer</th>"
                  + "<th>" + (current.use_points ? "Place" : "Time") + "</th>"
                  + "</tr>"
                  + "</table>"));
  if (holding) {
    heat_lineup
      .append("<div class='now-racing-spacer'/>")
      .append($("<div class='staging'/>")
              .append(
                "<table>"
                  + "<tr>"
                  + "<th colspan='2'>Staging</th>"
                  + "</tr>"
                  + "</table>"));
  }
  var racers_table = $("#now-racing-group table").first();
  var next_table = $("#now-racing-group table").slice(1);

  if (!holding) {
    g_current_heat.roundid = current.roundid;
    g_current_heat.heat = current.heat;
    g_current_heat.hold_until = 0;
    g_current_heat_racers = new_racers;
    racers = new_racers;
  }

  for (var lane = 1; lane <= nlanes; ++lane) {
    var r = racers.find(function(rr) { return rr.lane == lane; });
    var hr = false;
    var nr = false;
    if (holding) {
      hr = current.heat_results.find(function(h) { return h.lane == lane; });
      nr = new_racers.find(function(n) { return n.lane == lane; });
    }

    var result = "";
    if (r) {
      result = current.use_points ? r.finishplace : r.finishtime;
    }
    if (holding && hr) {
      result = current.use_points ? hr.place : hr.time;
    }
    racers_table.append('<tr><td>' + lane + '</td>'
                        + '<td>' + (r ? r.carnumber : '') + '</td>'
                        + '<td class="racer-name">' + (r ? r.name : '') + '</td>'
                        + '<td>' + result + '</td>'
                        + '</tr>');
    if (holding) {
      next_table.append('<tr>'
                        + '<td>' + (nr ? nr.carnumber : '&nbsp;') + '</td>'
                        + '<td class="racer-name">' + (nr ? nr.name : '') + '</td>'
                        + '</tr>');
    }
  }
}

// For master scheduling, make a synthetic round data structure by calculating
// totals across all the rounds records for the highest round number.  (I.e., if
// there are round 2 rounds, only total them and exclude the round 1 rounds.)
function calculate_totals(rounds) {
  var max_round = -1;
  for (var i = 0; i < rounds.length; ++i) {
    max_round = Math.max(max_round, rounds[i].round);
  }
  
  var total_roster_size = 0;
  var total_passed = 0;
  var total_racers_scheduled = 0;
  var total_heats_scheduled = 0;
  var total_heats_run = 0;
  for (var i = 0; i < rounds.length; ++i) {
    var round = rounds[i];
    if (round.round == max_round) {
      total_roster_size += round.roster_size;
      total_passed += round.passed;
      total_racers_scheduled += round.passed - round.unscheduled;
      total_heats_scheduled += round.heats_scheduled;
      total_heats_run += round.heats_run;
    }
  }
  return {round: max_round,
          roster_size: total_roster_size,
          passed: total_passed,
          scheduled: total_racers_scheduled,
          heats_scheduled: total_heats_scheduled,
          heats_run: total_heats_run};
}

function offer_new_rounds(rounds, classes) {
  if (g_new_round_modal_open) {
    console.log("Skipping offer_new_rounds because g_new_round_modal_open is set");
    return;
  }

  // What we really want to look at is the highest round for each
  // class, and then decide whether that round was completed or not.

  var highest_rounds = {};  // { classid => highest_round }
  for (var i = 0; i < rounds.length; ++i) {
    var round = rounds[i];
    var classid = round.classid;
    if (highest_rounds[classid] ? highest_rounds[classid].round < round.round : true) {
      highest_rounds[classid] = round;
    }
  }

  var completed_rounds = [];
  for (var classid in highest_rounds) {
    var round = highest_rounds[classid];
    if (round.heats_scheduled > 0 && round.heats_scheduled == round.heats_run) {
      var cl = find_by_classid(classes, classid);
      round.subgroups = cl.subgroups;
      completed_rounds.push(round);
    }
  }

  g_completed_rounds = completed_rounds;

  // Show or hide the block with the "Add New Rounds" button
  $("#add-new-rounds-button").toggleClass("hidden", completed_rounds.length == 0);
}

function process_coordinator_poll_json(json) {
  json['current-heat'].heat_results = [];
  for (var i = 0; i < json['heat-results'].length; ++i) {
    json['current-heat'].heat_results.push(json['heat-results'][i]);
  }
  $("#now-racing-group-buttons").empty();
  update_for_current_round(json['current-heat']);
  update_for_last_heat(json);

  $("#start_race_button_div").toggleClass('hidden',
                                          !json['timer-state']['remote-start']);

  g_ready_aggregate_classes = json['ready-aggregate'];

  for (var i = 0; i < json.rounds.length; ++i) {
    // May get changed to now-racing for the current round
    json.rounds[i].category = 
      json.rounds[i].heats_scheduled > json.rounds[i].heats_run
      ? 'ready-to-race'
      : json.rounds[i].heats_run > 0
      ? 'done-racing'
      : 'not-yet-scheduled';
  }

  offer_new_rounds(json.rounds, json['classes']);

  $("#playlist-group").toggleClass('hidden', json['current-heat'].use_master_sched);

  if (json['current-heat'].use_master_sched) {
    var totals = calculate_totals(json.rounds);
    totals.roundid = -1;
    totals['class'] = totals.name = "Interleaved Schedule";
    totals.category = "master-schedule";

    if ($("#master-schedule-group .control_group").length == 0) {
      $("#master-schedule-group").empty();
      generate_scheduling_control_group(totals, json['current-heat'], json['timer-state']);
    } else {
      inject_into_scheduling_control_group(totals, json['current-heat'], json['timer-state']);
    }
    $("#schedule-and-race").addClass('hidden');
    $("#schedule-only").val("Make Schedule");
  } else {
    $("#master-schedule-group").empty();
    $("#schedule-and-race").removeClass('hidden');
    $("#schedule-only").val("Schedule Only");
  }
  // The "Schedule + Race" option from the #schedule_modal shouldn't be offered
  // if we're in interleaved heats:
  $("#schedule-and-race").toggleClass('hidden', json['current-heat'].use_master_sched);

  var layout = {'now-racing': [],
                'ready-to-race': [],
                'not-yet-scheduled': [],
                'done-racing': [],
               };
  $.each(json.rounds, function (index, round) {
    if (round.roundid == json['current-heat'].roundid) {
      json['current-heat'].heats_scheduled = round.heats_scheduled;
      round.category = 'now-racing';
    }
    layout[round.category].push(round.roundid);
  });

  // matched will be true if the number and order of rounds hasn't
  // changed from what's displayed.
  var matched = true;
  for (var key in layout) {
    if (layout[key].toString() != g_rounds_layout[key].toString()) {
      matched = false;
    }
  }

  if (matched) {
    // TODO Want to remove everything except data-name="buttons"
    $.each(json.rounds, function (index, round) {
      inject_into_scheduling_control_group(round, json['current-heat'], json['timer-state']);
    });
  } else {
    g_rounds_layout = layout;
    $(".scheduling_control_group").empty();
    g_aggregate_rounds = [];
    $.each(json.rounds, function (index, round) {
      if (round.aggregate) {
        g_aggregate_rounds.push(round.roundid);
      }
      if (round.roundid == json['current-heat'].roundid) {
        generate_current_round_control_group(round, json['current-heat'], json['timer-state']);
      } else {
        generate_scheduling_control_group(round, json['current-heat'], json['timer-state']);
      }
    });
  }

  if (json['current-heat'].use_master_sched) {
    $("div.master_heat_reminder").remove();
    $.each(json.rounds, function (index, round) {
      // console.log(round);
      if (round.heats_scheduled == 0) {
        $("<div class='master_heat_reminder'></div>")
          .append("<span class='master_heat_msg'>&nbsp;Not scheduled&nbsp;</span>")
          .append("<img src='img/trouble_triangle_small.png'/>")
          .prependTo(
          $("div.control_group.scheduling_control[data-roundid=" + round.roundid + "]"));
      }
    });
  }

  generate_timer_state_group(json['timer-state']);

  generate_replay_state_group(json['replay-state']);

  generate_current_heat_racers(json.racers, json['current-heat'], json['timer-state'].lanes);

  if (json['current-heat'].roundid == -100 && json['current-heat'].now_racing) {
    $("#now-racing-group")
      .empty()
      .append($("<h3 id='timer-testing-herald'>Simulated racing in progress</h3>")
              .append("<input class='stop-test' type='button'"
                      + " onclick='handle_stop_testing();' value='Stop'/>"));
  } else if (json['refused-results'] > 0) {
    $("#now-racing-group")
          .empty()
          .append($("<div id='timer-testing-herald'></div>")
                  .append($("<h3></h3>")
                          .append($("<span></span>").text(json['refused-results']))
                          .append(json['refused-results'] == 1
                                  ? " unexpected result from timer has been received."
                                  : " unexpected results from timer have been received.")));
  } else {
    $('#timer-testing-herald').remove();
  }

  $("#not-racing-warning").toggleClass('hidden', json['current-heat']['now_racing']);;

  $("#playlist-start").toggleClass('hidden',
                                   !(json['current-heat'].roundid == -1 &&
                                     json.rounds.some(r => r['next-round'])));

  // Hide the control group if there's nothing to show
  $("#supplemental-control-group").toggleClass("hidden",
                                               $("#add-new-rounds-button").hasClass("hidden") &&
                                               $("#now-racing-group-buttons").is(":empty"));
}

function coordinator_poll() {
  if (typeof(phantom_testing) == 'undefined' || !phantom_testing) {
    $.ajax(g_action_url,
           {type: 'GET',
            data: {query: 'poll.coordinator',
                   roundid: g_current_heat.roundid,
                   heat: g_current_heat.heat},
            success: function(json) {
              if (typeof(phantom_testing) == 'undefined' || !phantom_testing) {
                process_coordinator_poll_json(json);
              }
            },
           });
  } else {
    console.log("NOT polling");
  }
}


$(function() {
  setInterval(coordinator_poll, 2000);
  coordinator_poll();
});
