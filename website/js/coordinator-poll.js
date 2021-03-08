// Requires dashboard-ajax.js

// Data structure describing each round:
//
// round = { roundid:
//           classname:
//           roundname:
//           round: (number)
//           roster_size, racers_passed, racers_unscheduled, racers_scheduled,
//           heats_scheduled, heats_run
//           category: one of ("now-racing", "master-schedule",
//                             "ready-to-race", "not-yet-scheduled",
//                             "done-racing",)
//         }
// (racers_passed = racers_scheduled + racers_unscheduled)
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
//    {classid, classname, round, roundid, roundname, aggregate, category, subgroups,
//     heats_run,heats_scheduled,racers_passed,racers_scheduled,racers_unscheduled,roster_size}
g_completed_rounds = [];

// Roundids of an aggregate rounds
g_aggregate_rounds = [];

// Parsing for poll.coordinator output

/* <current-heat now-racing= use-master-sched= use-points=
                 classid= roundid= round= group= heat= /> */
function parse_current_heat(data) {
  var current_xml = data.getElementsByTagName("current-heat")[0];
  if (!current_xml) {
    return false;
  }

  // NOTE: heats_scheduled gets written in process_coordinator_poll_response
  var current = {roundid: current_xml.getAttribute('roundid'),
                 heat: current_xml.getAttribute('heat'),
                 is_racing: current_xml.getAttribute('now-racing') == '1',
                 master_schedule: current_xml.getAttribute('use-master-sched') == '1',
                 use_points: current_xml.getAttribute('use-points') == '1',
                 heat_results: []};

  var heat_results = data.getElementsByTagName('heat-result');
  for (var i = 0; i < heat_results.length; ++i) {
    current.heat_results.push({lane: heat_results[i].getAttribute('lane'),
                               time: heat_results[i].getAttribute('time'),
                               place: heat_results[i].getAttribute('place')});
  }
  
  return current;
}

function parse_rounds(data) {
    var rounds_xml = data.getElementsByTagName("round");
    var rounds = new Array(rounds_xml.length);
    for (var i = 0; i < rounds_xml.length; ++i) {
        var round_xml = rounds_xml[i];
        rounds[i] = {roundid: 1*round_xml.getAttribute('roundid'),
                     classid: 1*round_xml.getAttribute('classid'),
                     classname: round_xml.getAttribute('class'),
                     roundname: round_xml.textContent,
                     aggregate: round_xml.getAttribute('aggregate') == "" ? 0
                              : round_xml.getAttribute('aggregate'),
                     round: 1*round_xml.getAttribute('round'),
                     roster_size: 1*round_xml.getAttribute('roster_size'),
                     racers_passed: 1*round_xml.getAttribute('passed'),
                     racers_unscheduled: 1*round_xml.getAttribute('unscheduled'),
                     racers_scheduled: round_xml.getAttribute('passed') - round_xml.getAttribute('unscheduled'),
                     heats_scheduled: 1*round_xml.getAttribute('heats_scheduled'),
                     heats_run: 1*round_xml.getAttribute('heats_run'),
                     next: round_xml.hasAttribute('next_round'),
                     category: 'unassigned'};
        rounds[i].category = 
          // May get changed to now-racing for the current round
          rounds[i].heats_scheduled > rounds[i].heats_run ? 'ready-to-race' :
          rounds[i].heats_run > 0 ? 'done-racing' : 'not-yet-scheduled';
    }
    return rounds;
}

function parse_timer_state(data) {
    var tstate_xml = data.getElementsByTagName("timer-state")[0];
    if (tstate_xml) {
        return {status: tstate_xml.textContent,
                icon: tstate_xml.getAttribute('icon'),
                lanes: tstate_xml.getAttribute('lanes'),
                remote_start: tstate_xml.getAttribute('remote_start')};
    }
}

function parse_replay_state(data) {
    var replay_state_xml = data.getElementsByTagName("replay-state")[0];
    return {status: replay_state_xml.textContent,
            icon: replay_state_xml.getAttribute('icon'),
            connected: replay_state_xml.getAttribute('connected')};
}

function parse_racers(data) {
    var racers_xml = data.getElementsByTagName("racer");
    var racers = new Array(racers_xml.length);
    for (var i = 0; i < racers_xml.length; ++i) {
        racers[i] = {lane: racers_xml[i].getAttribute("lane"),
                     name: racers_xml[i].getAttribute("name"),
                     carnumber: racers_xml[i].getAttribute("carnumber"),
                     finishtime: racers_xml[i].getAttribute("finishtime"),
                     finishplace: racers_xml[i].getAttribute("finishplace")};
    }
    return racers;
}

function parse_ready_aggregate_classes(data) {
  ready_aggregate_classes = [];
  var elts = data.getElementsByTagName("ready-aggregate");
  for (var i = 0; i < elts.length; ++i) {
    ready_aggregate_classes[i] =
      {classid: elts[i].getAttribute("classid"),
       classname: elts[i].textContent,
       'by-subgroup': elts[i].getAttribute("by-subgroup") != 0};
  }
  return ready_aggregate_classes;
}

// <class classid="4" count="16" nrounds="1" ntrophies="-1" name="Webelos (&quot;Webes">
//    <rank rankid="4" count="16" name="Webelos (&quot;Webes"/>
function parse_classes(data) {
  var elts = data.getElementsByTagName("class");
  var classes = {};
  for (var i = 0; i < elts.length; ++i) {
    var rank_elts = elts[i].getElementsByTagName("rank");
    var ranks = new Array(rank_elts.length);
    for (var ri = 0; ri < rank_elts.length; ++ri) {
      ranks[ri] = {rankid: rank_elts[ri].getAttribute('rankid'),
                   name: rank_elts[ri].getAttribute('name')};
    }
    classes[elts[i].getAttribute('classid')] = {
                  name: elts[i].getAttribute('name'),
                  subgroups: ranks};
  }
  return classes;
}

function update_for_last_heat(data, racers) {
  var last_heat = data.getElementsByTagName("last_heat");
  var button = $("#rerun-button");
  var rerun_type = last_heat.length == 0 ? 'none' : last_heat[0].getAttribute("type");
  var enable = true;
  button.prop("data-rerun", rerun_type);
  if (rerun_type == 'recoverable') {
    button.val("Reinstate Heat");
  } else {
    var results = false;
    for (var i = 0; i < racers.length; ++i) {
      if (racers[i]['finishtime'] || racers[i]['finishplace']) {
        results = true;
      }
    }
    if (results) {
      button.val("Re-Run This Heat");
      button.prop("data-rerun", 'current');
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
    if (isracing_checkbox.prop('checked') != current.is_racing) {
        isracing_checkbox.prop('checked', current.is_racing);
        g_updating_current_round = true;
        try {
            isracing_checkbox.trigger("change", true);
        } finally {
            g_updating_current_round = false;
        }
    }
}

function generate_timer_state_group(tstate) {
    $("#timer_status_text").text(tstate.status);
    $("#timer_status_icon").attr('src', tstate.icon);
    if (tstate.lanes != '' && tstate.lanes > 0) {
        $("#lane_count").text(tstate.lanes);
    }
}

function generate_replay_state_group(replay_state) {
    $("#replay_status").text(replay_state.status);
    $("#replay_status_icon").attr('src', replay_state.icon);
    if (replay_state.connected) {
        $("#test_replay").removeClass("hidden");
    } else {
        $("#test_replay").addClass("hidden");
    }
}

// Updates progress bars with new progress values
function inject_progress_bars(control_group, round) {
    if (round.roster_size > 0) {
        // bar2 (yellow) = passed
        // bar1 (blue) = scheduled
        // Trouble here will be scheduled-but-no-longer-passed, i.e., scheduled > passed
        var passed = Math.max(round.racers_passed, round.racers_scheduled);
        control_group.find(".racers .bar2").width((100 * passed / round.roster_size) + '%');
        if (passed > 0) {
            control_group.find(".racers .bar1").width((100 * round.racers_scheduled / passed) + '%');
        }
    }

    if (round.heats_scheduled > 0) {
        control_group.find(".heats .bar1").width((100 * round.heats_run / round.heats_scheduled) + '%');
    }
}

// Injects new progress values into the progress text
function inject_progress_text(control_group, round) {
    control_group.find("[data-name=roster_size]").text(round.roster_size);
    control_group.find("[data-name=n_passed]").text(round.racers_passed);
    control_group.find("[data-name=scheduled]").text(round.racers_scheduled);
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
//           classname:
//           round: (number)
//           roster_size, racers_passed, racers_scheduled, heats_scheduled, heats_run
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
              .text(round.roundname)
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

  if (round.next) {
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
              .text(round.roundname))
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

  if (round.roundid == -1) {
    buttons.append('<input type="button"'
                   + ' onclick="handle_master_next_up()" value="Next Up"/>');
  } else {
    if (round.heats_scheduled > 0 && round.heats_run == 0) {
      buttons.append('<input type="button"'
                     + ' onclick="handle_unschedule_button(' + round.roundid
                     + ', \'' + round.classname.replace(/"/g, '&quot;').replace(/'/, "\\'") + '\', '
                     + round.round + ')"'
                     + ' value="Unschedule"/>');
    } else if (round.racers_unscheduled > 0) {
      if (round.heats_run == 0) {
        if (timer_state.lanes != '' && timer_state.lanes > 0) {
          buttons.append('<input type="button"'
                         + ' onclick="show_schedule_modal(' + round.roundid + ')"'
                         + ' value="Schedule"/>');
        } else {
          buttons.append("<p>Can't schedule heats, because the number of lanes hasn\'t" +
                         " been determined.<br/>" +
                         "Enter the number of lanes on the <a href='settings.php'>Settings</a> page.</p>");
        }
      } else if (false /* TODO: Reschedule is not ready for prime time */) {
        buttons.append('<input type="button"' 
                       + ' onclick="handle_reschedule_button(' + round.roundid + ')"'
                       + ' value="Reschedule"/>');
      }
    }

    if (round.heats_scheduled == 0 && round.heats_run == 0 &&
        (round.round > 1 || round.aggregate)) {
      buttons.append('<input type="button"'
                     + ' onclick="handle_delete_round_button(' + round.roundid
                     + ', \'' + round.classname.replace(/"/g, '&quot;').replace(/'/, "\\'") + '\', '
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
  var total_racers_passed = 0;
  var total_racers_scheduled = 0;
  var total_heats_scheduled = 0;
  var total_heats_run = 0;
  for (var i = 0; i < rounds.length; ++i) {
    var round = rounds[i];
    if (round.round == max_round) {
      total_roster_size += round.roster_size;
      total_racers_passed += round.racers_passed;
      total_racers_scheduled += round.racers_scheduled;
      total_heats_scheduled += round.heats_scheduled;
      total_heats_run += round.heats_run;
    }
  }
  return {round: max_round,
          roster_size: total_roster_size,
          racers_passed: total_racers_passed,
          racers_scheduled: total_racers_scheduled,
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
      round.subgroups = classes[classid].subgroups;
      completed_rounds.push(round);
    }
  }

  g_completed_rounds = completed_rounds;

  // Show or hide the block with the "Add New Rounds" button
  $("#add-new-rounds-button").toggleClass("hidden", completed_rounds.length == 0);
}

function process_coordinator_poll_response(data) {
  var timer_state = parse_timer_state(data);
  var current = parse_current_heat(data);
  if (!current) {
    console.log("Returning early because no current heat");
    return;
  }
  $("#now-racing-group-buttons").empty();
  update_for_current_round(current);
  var racers = parse_racers(data)
  update_for_last_heat(data, racers);

  $("#start_race_button_div").toggleClass('hidden',
                                          timer_state.remote_start != "1");

  g_ready_aggregate_classes = parse_ready_aggregate_classes(data);

  var classes = parse_classes(data);
  var rounds = parse_rounds(data);
  offer_new_rounds(rounds, classes);

  if (current.master_schedule) {
    var totals = calculate_totals(rounds);
    totals.roundid = -1;
    totals.roundname = totals.classname = "Interleaved Schedule";
    totals.category = "master-schedule";

    if ($("#master-schedule-group .control_group").length == 0) {
      $("#master-schedule-group").empty();
      generate_scheduling_control_group(totals, current, timer_state);
    } else {
      inject_into_scheduling_control_group(totals, current, timer_state);
    }
    $("#schedule-and-race").addClass('hidden');
    $("#schedule-only").val("Make Schedule");
  } else if (!current.master_schedule) {
    $("#master-schedule-group").empty();
    $("#schedule-and-race").removeClass('hidden');
    $("#schedule-only").val("Schedule Only");
  }
  // The "Schedule + Race" option from the #schedule_modal shouldn't be offered
  // if we're in interleaved heats:
  $("#schedule-and-race").toggleClass('hidden', current.master_schedule);

  var layout = {'now-racing': [],
                'ready-to-race': [],
                'not-yet-scheduled': [],
                'done-racing': [],
               };
  $.each(rounds, function (index, round) {
    if (round.roundid == current.roundid) {
      current.heats_scheduled = round.heats_scheduled;
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
    $.each(rounds, function (index, round) {
      inject_into_scheduling_control_group(round, current, timer_state);
    });
  } else {
    g_rounds_layout = layout;
    $(".scheduling_control_group").empty();
    g_aggregate_rounds = [];
    $.each(rounds, function (index, round) {
      if (round.aggregate) {
        g_aggregate_rounds.push(round.roundid);
      }
      if (round.roundid == current.roundid) {
        generate_current_round_control_group(round, current, timer_state);
      } else {
        generate_scheduling_control_group(round, current, timer_state);
      }
    });
  }

  generate_timer_state_group(timer_state);

  generate_replay_state_group(parse_replay_state(data));

  generate_current_heat_racers(racers, current, timer_state.lanes);

  if (current.roundid == -100 && current.is_racing) {
    $("#now-racing-group")
      .empty()
      .append($("<h3 id='timer-testing-herald'>Simulated racing in progress</h3>")
              .append("<input class='stop-test' type='button'"
                      + " onclick='handle_stop_testing();' value='Stop'/>"));
  } else {
    $('#timer-testing-herald').remove();
  }

  $("#playlist-start").toggleClass('hidden',
                                   !(current.roundid == -1 && rounds.some(r => r.next)));

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
            success: function(data) {
              if (typeof(phantom_testing) == 'undefined' || !phantom_testing) {
                process_coordinator_poll_response(data);
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
