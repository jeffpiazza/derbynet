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
                success: function(data) { process_coordinator_poll_response(data); }
               });
    }
}
$(function() { $("#is-currently-racing").on("change", handle_isracing_change); });

function handle_skip_heat_button() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.select',
                   heat: 'next'},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function handle_previous_heat_button() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.select',
                   heat: 'prev'},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function handle_rerun(button) {
  var rerun_type = $(button).prop('data-rerun');
  // rerun_type values are: 'none', recoverable, available, current
  if (rerun_type == 'current' || rerun_type == 'available') {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.rerun',
                   heat: rerun_type == 'current' ? 'current' : 'last'},
            success: function(data) { process_coordinator_poll_response(data); }
           });
  } else if (rerun_type = 'recoverable') {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.reinstate'},
            success: function(data) { process_coordinator_poll_response(data); }
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

function show_manual_results_modal() {
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
                           + "<td><input type='text' size='8'"
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
            success: function(data) { process_coordinator_poll_response(data); }
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
            success: function(data) { process_coordinator_poll_response(data); }
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
              process_coordinator_poll_response(data);
              if (then_race && data.getElementsByTagName('success').length > 0) {
                handle_race_button(roundid);
              }
            }
           });
}

function handle_reschedule_button(roundid) {
    // TODO: On success... 
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'schedule.reschedule',
                   roundid: roundid}});
}

function handle_race_button(roundid) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'heat.select',
                   roundid: roundid,
                   heat: 1,
                   now_racing: 1},
            success: function(data) { process_coordinator_poll_response(data); }
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
                success: function(data) { process_coordinator_poll_response(data); }
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
                success: function(data) { process_coordinator_poll_response(data); }
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
            success: function(data) { process_coordinator_poll_response(data); }
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
    // of buttons with their own actions
    show_modal("#choose_new_round_modal", function(event) {
        return false;
    });
}

function handle_new_round_chosen(roundid) {
    close_modal_leave_background("#choose_new_round_modal");
    show_new_round_modal(roundid);
}

function show_new_round_modal(roundid) {
    $(".multi_den_only").addClass("hidden");
    $(".single_den_only").removeClass("hidden");
    $("#new_round_modal").removeClass("wide_modal");
    $("#new_round_modal #new_round_roundid").val(roundid);
    // For a single-round follow-on round, #bucketed flipswitch won't even be
    // present unless we're using subgroups.
    var bucketed = $("#new_round_modal #bucketed_single");
    if (bucketed) {
      bucketed.prop('checked', false)
        .trigger("change", true)
        .toggleClass('hidden', g_aggregate_rounds.includes(roundid));
    }

    show_modal("#new_round_modal", function(event) {
        handle_new_round_submit();
        return false;
    });
}

function handle_new_round_submit(roundid) {
    close_modal("#new_round_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: $("#new_round_modal form").serialize(),
            success: function(data) {
              process_coordinator_poll_response(data); }
           });
}

function handle_aggregate_chosen() {
    close_modal_leave_background("#choose_new_round_modal");
    show_aggregate_modal();
}

function show_aggregate_modal() {
    $(".multi_den_only").removeClass("hidden");
    $(".single_den_only").addClass("hidden");
    $("#new_round_modal").addClass("wide_modal");
    // Have to suspend updates to this dialog while it's open
    g_new_round_modal_open = true;
    show_modal("#new_round_modal", function(event) {
        handle_aggregate_submit();
        return false;
    });
}

function handle_aggregate_submit() {
    g_new_round_modal_open = false;
    close_modal("#new_round_modal");

    console.log($("#new_round_modal form").serialize());

    $.ajax(g_action_url,
           {type: 'POST',
            data: $("#new_round_modal form").serialize(),
            success: function(data) { process_coordinator_poll_response(data); }
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
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function handle_stop_testing() {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'timer.test',
                 'test-mode': 0},
         });
}

function populate_new_round_modals() {
  var completed_rounds = g_completed_rounds.slice(0);  // Copy the array

  var add_aggregate = completed_rounds.length > 1;
  var modal = $("#choose_new_round_modal");
  modal.empty();
  var multi_flipswitches = $("#multi_flipswitches");
  multi_flipswitches.empty();
  while (completed_rounds.length > 0) {
    var roundno = completed_rounds[0].round;
    modal.append('<h3>Add Round ' + (roundno + 1) + '</h3>');
    var i = 0;
    while (i < completed_rounds.length) {
      if (completed_rounds[i].round == roundno) {
        var round = completed_rounds[i];
        var button = $('<input type="button" data-enhanced="true"/>');
        button.prop('value', round.classname);
        // Although syntactically it looks like a new round variable is created
        // each time through the loop, it's actually just one variable that's
        // reused/assigned each time.  Capturing that reused variable in the on-click
        // function won't work, so, we need to record the current roundid on
        // the button itself.
        button.prop('data-roundid', round.roundid);
        button.on('click', function(event) {
          handle_new_round_chosen($(this).prop('data-roundid'));
        });
        modal.append(button);

        var flipswitch_div = $('<div class="flipswitch-div"></div>');
        var label = $('<label for="roundid_' + round.roundid + '"'
                      + ' class="aggregate-label"'
                      + '></label>');
        label.text(round.classname);
        flipswitch_div.append(label);
        flipswitch_div.append(wrap_flipswitch($('<input type="checkbox"'
                                                + ' id="roundid_' + round.roundid + '"'
                                                + ' name="roundid_' + round.roundid + '"'
                                                + ' checked="checked"/>')));
        multi_flipswitches.append(flipswitch_div);
        completed_rounds.splice(i, 1);
      } else {
        ++i;
      }
    }
  }
  if (add_aggregate) {
    modal.append('<h3>Add Aggregate Round</h3>');
    var button = $('<input type="button" data-enhanced="true" value="Aggregate Round"/>');
    button.on('click', function(event) { handle_aggregate_chosen(); });
    modal.append(button);
    // Even though we're doing the embellishment explicitly, the create
    // trigger is still needed to make the flipswitches actually do
    // anything.
    multi_flipswitches.trigger("create");
  }
  modal.append('<h3>&nbsp;</h3>');
  modal.append('<input type="button" data-enhanced="true" value="Cancel"'
               + ' onclick=\'close_modal("#choose_new_round_modal");\'/>');
}
