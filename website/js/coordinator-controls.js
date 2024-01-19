// Requires dashboard-ajax.js

// g_current_heat_racers remembers who the racers in the current heat
// are, so the modal for manual heat results can be populated when
// displayed.
g_current_heat_racers = new Array();

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
g_completed_rounds = [];

// Roundids of an aggregate rounds
g_aggregate_rounds = [];
// {classid:, class:, by-subgroup:} for each aggregate class for which a
// first round could be created
g_ready_aggregate_classes = [];

// Controls for current racing group:

function handle_start_race_button() {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'timer.remote-start'}
         });
}

function handle_isracing_change(event, scripted) {
    if (!g_updating_current_round) {
        $.ajax(g_action_url,
               {type: 'POST',
                data: {action: 'heat.select',
                       now_racing: $("#is-currently-racing").prop('checked') ? 1 : 0},
                success: function(json) { process_coordinator_poll_json(json); }
               });
    }
}
$(function() { $("#is-currently-racing").on("change", handle_isracing_change); });

function handle_skip_heat_button() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.select',
                   heat: 'next'},
            success: function(json) { process_coordinator_poll_json(json); }
           });
}

function handle_previous_heat_button() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.select',
                   heat: 'prev'},
            success: function(json) { process_coordinator_poll_json(json); }
           });
}

function handle_rerun(button) {
  var rerun_type = $(button).attr('data-rerun');
  // rerun_type values are: 'none', recoverable, available, current
  if (rerun_type == 'current' || rerun_type == 'available') {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.rerun',
                   heat: rerun_type == 'current' ? 'current' : 'last'},
            success: function(data) { process_coordinator_poll_json(data); }
           });
  } else if (rerun_type = 'recoverable') {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.reinstate'},
            success: function(data) { process_coordinator_poll_json(data); }
           });
  }
}

// Controls for Replay
function handle_test_replay() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'replay.test'}
           });
}

function trigger_replay() {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'replay.trigger'}
         });
}

// Present the modal for entering manual heat results.
//
// If should_trigger_replay is true, and there aren't any existing results for
// this heat, then trigger a replay event as we're present the manual results
// modal.
function on_manual_results_button_click(should_trigger_replay) {
    // g_current_heat_racers: lane, name, carnumber, finishtime, finishplace
    var racer_table = $("#manual_results_modal table");
    racer_table.empty();
  // TODO Show place instead of time if using points
    racer_table.append("<tr><th>Lane</th><th>Racer</th><th>Car</th><th>Time</th></tr>");
    var any_results = false;
    for (var i = 0; i < g_current_heat_racers.length; ++i) {
        var racer = g_current_heat_racers[i];
        racer_table.append("<tr><td>" + racer.lane + "</td>"
                           + "<td>" + racer.name + "</td>"
                           + "<td>" + racer.carnumber + "</td>"
                           + "<td><input class='lane-time' type='number' step='0.00001'"
                               + " name='lane" + racer.lane + "'" 
                               + " value='" + racer.finishtime + "'/>"
                           + "</td>"
                           + "</tr>");
        if (racer.finishtime) {
            any_results = true;
        }
    }

    if (any_results) {
      $("#discard-results").removeClass("hidden");
    } else {
      if (should_trigger_replay) {
        trigger_replay();
      }
      $("#discard-results").addClass("hidden");
    }

    show_modal("#manual_results_modal", function(event) {
        handle_manual_results_submit();
        return false;
    });
}

function handle_manual_results_submit( ) {
    close_modal("#manual_results_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: $("#manual_results_modal form").serialize(),
            success: function(data) { process_coordinator_poll_json(data); }
           });
}

function handle_discard_results_button() {
    close_modal("#manual_results_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            // TODO: There's a risk that the coordinator form gets
            // grossly out of sync with the database, such that the
            // identification of 'current' heat is different from what
            // the user chose.  If current-heat changes while
            // manual-results dialog is open, maybe close the dialog
            // and start over?
            data: {action: 'result.delete',
                   roundid: 'current',
                   heat: 'current'},
            success: function(data) { process_coordinator_poll_json(data); }
           });
}

// Controls for racing rounds

function show_schedule_modal(roundid) {
  show_modal("#schedule_modal", function(event) {
    handle_schedule_submit(roundid,
                           $("#schedule_num_rounds").val(),
                           $("input[clicked='true']",
                             $(event.target)).attr("data-race") == 'true');
    return false;
  });
}

// There are two different submit buttons for the schedule_modal, depending on
// whether we want to start racing immediately or not.  mark_clicked() runs as an
// onclick handler so we can distinguish between the two submit buttons.
function mark_clicked(submit_js) {
  $("input[type=submit]", submit_js.parents("form")).removeAttr("clicked");
  submit_js.attr("clicked", "true");
  return true;  // Control now passes to show_schedule_modal's callback,
                // and then handle_schedule_submit
}

function handle_schedule_submit(roundid, n_times_per_lane, then_race) {
    close_modal("#schedule_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'schedule.generate',
                   roundid: roundid,
                   n_times_per_lane: n_times_per_lane},
            success: function(data) {
              process_coordinator_poll_json(data);
              if (then_race && data.outcome.summary == "success") {
                handle_race_button(roundid);
              }
            }
           });
}

function handle_reschedule_button(roundid) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'schedule.reschedule',
                   roundid: roundid},
            success: function(json) { process_coordinator_poll_json(json); }
           });
}

function handle_race_button(roundid) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.select',
                   roundid: roundid,
                   heat: 1,
                   now_racing: 1},
            success: function(json) { process_coordinator_poll_json(json); }
            });
}

function handle_unschedule_button(roundid, classname, round) {
    $("#unschedule_round").text(round);
    $("#unschedule_class").text(classname);
    show_modal("#unschedule_modal", function(event) {
        close_modal("#unschedule_modal");
        $.ajax(g_action_url,
               {type: 'POST',
                data: {action: 'schedule.unschedule',
                       roundid: roundid},
                success: function(data) { process_coordinator_poll_json(data); }
               });
        return false;
    });
}

function handle_delete_round_button(roundid, classname, round) {
    $("#delete_round_round").text(round);
    $("#delete_round_class").text(classname);
    show_modal("#delete_round_modal", function(event) {
        close_modal("#delete_round_modal");
        $.ajax(g_action_url,
               {type: 'POST',
                data: {action: 'roster.delete',
                       roundid: roundid},
                success: function(data) { process_coordinator_poll_json(data); }
               });
        return false;
    });
}

function handle_make_changes_button(roundid) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.select',
                   roundid: roundid,
                   heat: 1,
                   now_racing: 0},
            success: function(json) { process_coordinator_poll_json(json); }
           });
}

function handle_purge_button(roundid, heats_run) {
  var control_group = $("div[data-roundid=\"" + roundid + "\"]");
  $("#purge_round_name").text(control_group.find(".roundclass").text());
  $("#purge_round_no").text(control_group.find(".roundno").text());
  $("#purge_results_count").text(heats_run);
  show_modal("#purge_modal", function(event) {
    close_modal("#purge_modal");
    show_secondary_modal("#purge_confirmation_modal", function(event) {
      close_secondary_modal("#purge_confirmation_modal");
      $.ajax('action.php',
           {type: 'POST',
            data: {action: 'database.purge',
                   purge: 'results',
                   roundid: roundid},
            success: function(data) { coordinator_poll(); }
           });
      return false;
    });
    return false;
  });
}

function show_choose_new_round_modal() {
  populate_new_round_modals();

  // There's no submit, or even a form, in this modal; just a bunch
  // of buttons with their own actions:
  //
  // - handle_new_round_follow_on to create a follow-on round to an existing round
  // - handle_new_round_aggregate_class to create a first round for an existing aggregate class
  // - handle_new_round_make_aggregate to create an aggregate round ("Grand Final")
  show_modal("#choose_new_round_modal", function(event) {
    return false;
  });
}

// When the "aggregate-by" checkbox changes, slide to hide or show
// constituent-rounds div
function on_aggregate_by_change() {
  $("#constituent-rounds").css('margin-left',
                               $("#aggregate-by-checkbox").is(':checked') ? '-500px' : '0px');
  update_bucketed_checkbox(!$("#aggregate-by-checkbox").is(':checked'));
}

function update_bucketed_checkbox(for_group) {
  $("#bucketed-checkbox")
    .attr('data-on-text',
          $("#bucketed-checkbox").attr(for_group ? 'data-group-text' : 'data-subgroup-text'));
  flipswitch_refresh($("#bucketed-checkbox"));
}


// Create a follow-on round to an existing round.
function handle_new_round_follow_on(roundid) {
  console.log('follow-on for roundid=' + roundid);  // TODO
  close_modal_leave_background("#choose_new_round_modal");
  $("#new-round-modal div").removeClass("hidden");
  $(".aggregate-only").addClass("hidden");
  $("#new-round-modal #new_round_roundid").val(roundid);

  update_bucketed_checkbox(/* for_group */ false);
  $("#new-round-modal #bucketed-checkbox")
    .prop('checked', false)
    .trigger("change", true)
    .toggleClass('hidden',
                 !g_use_subgroups &&
                 g_aggregate_rounds.includes(roundid));

  show_modal("#new-round-modal", function(event) {
    on_submit_new_round_follow_on(roundid);
    return false;
  });
}

function on_submit_new_round_follow_on(roundid) {
  close_modal("#new-round-modal");
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'roster.new',
                 roundid: roundid,
                 top: $("#new-round-top").val(),
                 bucketed: $("#bucketed-checkbox").prop('checked') ? 1 : 0},
          success: function(data) {
            process_coordinator_poll_json(data); }
         });
}

// Manufacture a new aggregate round (ephemeral aggregate class) from top
// finishers in several completed rounds.
function handle_new_round_make_aggregate() {
  close_modal_leave_background("#choose_new_round_modal");
  $("#new-round-modal div").removeClass("hidden");
  $("#aggregate-by-div").toggleClass('hidden', !g_use_subgroups);
  if (!g_use_subgroups) {
    $('#aggregate-by-checkbox').prop('checked', false);
  }

  update_bucketed_checkbox(/* for_group */ true);

  g_new_round_modal_open = true;
  show_modal("#new-round-modal", function(event) {
    on_submit_new_round_make_aggregate();
    return false;
  });
}

function on_submit_new_round_make_aggregate() {
  g_new_round_modal_open = false;
  close_modal("#new-round-modal");
  $.ajax(g_action_url,
         {type: 'POST',
          data:  'action=roster.new&' +
                 $("#new-round-common input").serialize() + '&' +
                 $("#agg-classname-div input").serialize() + '&' +
                ($("#aggregate-by-checkbox").is(':checked')
                 ? $("#constituent-subgroups input").serialize()
                 : $("#constituent-rounds input").serialize()),
          success: function(data) { process_coordinator_poll_json(data); }
         });
}

// Create first round for a pre-defined aggregate class
function handle_new_round_aggregate_class(classid, by_subgroup) {
  close_modal_leave_background("#choose_new_round_modal");
  $("#new-round-modal div").removeClass("hidden");
  $("div.for-choosing-constituents").addClass("hidden");
  $("#agg-classname-div").addClass("hidden");
  g_new_round_modal_open = true;

  update_bucketed_checkbox(/* for_group */ !by_subgroup);

  show_modal("#new-round-modal", function(event) {
    on_submit_new_round_aggregate_class(classid);
    return false;
  });
}

function on_submit_new_round_aggregate_class(classid) {
  close_modal("#new-round-modal");
  g_new_round_modal_open = false;

  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'roster.new',
                 classid: classid,
                 top: $("#new-round-top").val(),
                 bucketed: $("#bucketed-checkbox").prop('checked') ? 1 : 0},
          success: function(data) {
            process_coordinator_poll_json(data); }
         });
}

function show_replay_settings_modal() {
    show_modal("#replay_settings_modal", function(event) {
        handle_replay_settings_submit();
        return false;
    });
}

function handle_replay_settings_submit() {
    close_modal("#replay_settings_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: $("#replay_settings_modal form").serialize(),
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function handle_master_next_up() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.select',
                   heat: 'next-up',
                   now_racing: 0},
            success: function(json) { process_coordinator_poll_json(json); }
           });
}

function handle_stop_testing() {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'timer.test',
                 'test-mode': 0},
         });
}

function handle_start_playlist() {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'playlist.start'}
         });
}

function populate_new_round_modals() {
  // Each round in completed_rounds is the highest round for its class and all its heats have been run.
  var completed_rounds = g_completed_rounds.slice(0);  // Copy the array

  var add_aggregate = completed_rounds.length > 1;
  var modal = $("#choose_new_round_modal");
  modal.empty();
  var constituent_rounds_div = $("#constituent-rounds").empty();
  var constituent_subgroups_div = $("#constituent-subgroups").empty();
  while (completed_rounds.length > 0) {
    var roundno = completed_rounds[0].round;
    modal.append('<h3>Add Round ' + (roundno + 1) + '</h3>');
    var i = 0;
    while (i < completed_rounds.length) {
      if (completed_rounds[i].round == roundno) {
        var round = completed_rounds[i];
        console.log(round);
        // For completed rounds, offer a button to generate a follow-on round
        var button = $('<input type="button"/>');
        button.attr('value', round['class']);
        // Although syntactically it looks like a new round variable is created
        // each time through the loop, it's actually just one variable that's
        // reused/assigned each time.  Capturing that reused variable in the on-click
        // function won't work, so, we need to record the current roundid on
        // the button itself.
        button.attr('data-roundid', round.roundid);
        button.on('click', function(event) {
          handle_new_round_follow_on($(this).attr('data-roundid'));
        });
        modal.append(button);

        // A completed round can also be incorporated into a new aggregate round
        constituent_rounds_div.append(
          $('<div class="flipswitch-div"></div>')
            .append($('<label class="aggregate-label"/>')
                    .attr('for', 'classid_' + round.classid)
                    .text(round['class']))
            .append($('<input type="checkbox" class="flipswitch" checked="checked"/>')
                    .attr('id', 'classid_' + round.classid)
                    .attr('name', 'classid_' + round.classid)));

        // A completed round gives subgroups to choose from
        for (var ri = 0; ri < round.subgroups.length; ++ri) {
          var subgroup = round.subgroups[ri];
          
          constituent_subgroups_div.append(
            $('<div class="flipswitch-div"></div>')
              .append($('<label class="aggregate-label"/>')
                      .attr('for', 'rankid_' + subgroup.rankid)
                      .text(subgroup.name))
              .append($('<input type="checkbox" class="flipswitch" checked="checked"/>')
                      .attr('id', 'rankid_' + subgroup.rankid)
                      .attr('name', 'rankid_' + subgroup.rankid)));
        }
        
        completed_rounds.splice(i, 1);
      } else {
        ++i;
      }
    }
  }

  if (add_aggregate) {
    modal.append('<h3>Add Aggregate Round</h3>');
    for (var i = 0; i < g_ready_aggregate_classes.length; ++i) {
      var agg = g_ready_aggregate_classes[i];
      var button = $('<input type="button"/>');
      button.attr('value', agg['class']);
      button.attr('data-classid', agg.classid);
      button.attr('data-by-subgroup', agg['by-subgroup']);
      button.on('click', function(event) {
        handle_new_round_aggregate_class(
          $(this).attr('data-classid'),
          $(this).attr('data-by-subgroup'));
      });
      modal.append(button);

      // Aggregate rounds ready for scheduling can also be incorporated as
      // non-racing constituents for a new aggregate round.
      if (!g_ready_aggregate_classes[i]['by-subgroup']) {
        var id = 'classid_' + g_ready_aggregate_classes[i]['classid'];
        constituent_rounds_div.append(
          $('<div class="flipswitch-div"></div>')
            .append($('<label class="aggregate-label"/>')
                    .attr('for', id)
                    .text(g_ready_aggregate_classes[i]['class']))
            // leave these unchecked by default
            .append($('<input type="checkbox" class="flipswitch"/>')
                    .attr('id', id)
                    .attr('name', id)));
      }
    }
    var button = $('<input type="button" value="Aggregate Round"/>');
    button.on('click', function(event) { handle_new_round_make_aggregate(); });
    modal.append(button);

    flipswitch($("#constituent-div").find("input[type='checkbox']"));
  }
  
  modal.append('<h3>&nbsp;</h3>');
  modal.append('<input type="button" value="Cancel"'
               + ' onclick=\'close_modal("#choose_new_round_modal");\'/>');
}
