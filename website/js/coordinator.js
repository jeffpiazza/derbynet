
// TODO: This preamble copied from checkin.js; refactor!

g_action_url = "action.php";

// Note that this doesn't run if the $.ajax call has a success:
// callback that generates an error.
$(document).ajaxSuccess(function(event, xhr, options, xmldoc) {
	var fail = xmldoc.documentElement.getElementsByTagName("failure");

	if (fail && fail.length > 0) {
		alert("Action failed: " + fail[0].textContent);
	}
});

// <reload/> element
$(document).ajaxSuccess(function(event, xhr, options, xmldoc) {
	var reload = xmldoc.documentElement.getElementsByTagName("reload");
	if (reload && reload.length > 0) {
        console.log('ajaxSuccess event: reloading page');
		location.reload(true);
	}
});

$(document).ajaxError(function(event, jqXHR, ajaxSettings, thrownError) {
    console.log("ajaxError: " + thrownError);
});

// End of preamble

// g_current_heat_racers remembers who the racers in the current heat
// are, so the modal for manual heat results can be populated when
// displayed.
g_current_heat_racers = new Array();

// To avoid rewriting (as opposed to updating) round controls constantly,
// keep this signature that gives the roundids in each category, in order.
// We only need to rewrite the round controls when this signature changes.
g_rounds_layout = {'now-racing': ["force a first run"],
                   'ready-to-race': [],
                   'done-racing': [],
                   'not-yet-scheduled': []};

// The polling rate for the page is relatively fast, and each time
// causes a rewrite of everything for all the kiosks.  If the user has
// opened the <select> element for choosing a page to present on a
// kiosk, rewriting the control will cause the user's action to be
// ignored.  To combat this, we only update the kiosk groups when
// there's a detectable change, and for that, we keep a hash of what
// the last state of the kiosk data was.
g_kiosk_hash = 0;

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

function hash_string(hash, str) {
	for (i = 0; i < str.length; i++) {
		var ch = str.charCodeAt(i);
		hash = ((hash<<5)-hash)+ch;
		hash = hash & hash; // Convert to 32bit integer
	}
	return hash;
}

// Controls for current racing group:

function handle_isracing_change(event, scripted) {
    if (!g_updating_current_round) {
        $.ajax(g_action_url,
               {type: 'POST',
                data: {action: 'select-heat',
                       now_racing: $("#is-currently-racing").prop('checked') ? 1 : 0},
                success: function(data) { process_coordinator_poll_response(data); }
               });
    }
}
$(function() { $("#is-currently-racing").on("change", handle_isracing_change); });

function handle_skip_heat_button() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'select-heat',
                   heat: 'next'},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function handle_previous_heat_button() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'select-heat',
                   heat: 'prev'},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

// Controls for Replay
function handle_test_replay() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'replay-test'}
           });
}

// Controls for kiosks
// sel is the <select data-kiosk-address> input element
function handle_assign_kiosk_page_change(sel) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'assign-kiosk',
                   address: sel.getAttribute('data-kiosk-address'),
                   page: sel.value},
           });
}

function show_modal(modal_selector, submit_handler) {
    var modal_background = $("#modal_background");
    modal_background.css({'display': 'block',
                          'opacity': 0});
    modal_background.fadeTo(200, 0.5);

    var modal_div = $(modal_selector);
    var form = modal_div.find("form");
    form.off("submit");
    form.on("submit", submit_handler);

    var modal_width = modal_div.outerWidth();
    modal_div.removeClass("hidden");
    modal_div.css({ 
        'display': 'block',
        'position': 'fixed',
        'opacity': 0,
        'z-index': 11000,
        'left' : 50 + '%',
        'margin-left': -(modal_width/2) + "px",
        'top': 100 + "px"
    });
    modal_div.fadeTo(200, 1);
}

function close_modal(modal_selector) {
    $("#modal_background").fadeOut(200);
    $(modal_selector).addClass("hidden");
    $(modal_selector).css({'display': 'none'});
}

function show_kiosk_naming_modal(address, name) {
    $("#kiosk_name_field").val(name);
    show_modal("#kiosk_modal", function(event) {
        handle_name_kiosk(address, $("#kiosk_name_field").val());
        return false;
    });
}

function handle_name_kiosk(address, name) {
    close_modal("#kiosk_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'assign-kiosk',
                   address: address,
                   name: name},
           });
}


function show_manual_results_modal() {
    // g_current_heat_racers: lane, name, carnumber, finishtime
    var racer_table = $("#manual_results_modal table");
    racer_table.empty();
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
            data: {action: 'delete-results',
                   roundid: 'current',
                   heat: 'current'},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

// Controls for racing rounds

function show_schedule_modal(roundid) {
    show_modal("#schedule_modal", function(event) {
        handle_schedule_submit(roundid, $("#schedule_num_rounds").val());
        return false;
    });
}

function handle_schedule_submit(roundid, rounds) {
    close_modal("#schedule_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'schedule',
                   roundid: roundid,
                   nrounds: rounds},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function handle_reschedule_button(roundid) {
    // TODO: On success... 
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'reschedule',
                   roundid: roundid}});
}

function handle_race_button(roundid) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'select-heat',
                   roundid: roundid,
                   heat: 1,
                   now_racing: 1},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function handle_make_changes_button(roundid) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'select-heat',
                   roundid: roundid,
                   heat: 1,
                   now_racing: 0},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function show_choose_new_round_modal() {
    // There's no submit, or even a form, in this modal; just a bunch
    // of buttons with their own actions
    show_modal("#choose_new_round_modal", function(event) {
        return false;
    });
}

function handle_new_round_chosen(roundid) {
    close_modal("#choose_new_round_modal");
    show_new_round_modal(roundid);
}

function show_new_round_modal(roundid) {
    $(".multi_den_only").addClass("hidden");
    $(".single_den_only").removeClass("hidden");
    // TODO $("#new_round_modal").removeClass("wide_modal");
    $("#new_round_modal #new_round_roundid").val(roundid);
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
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function handle_grand_final_chosen() {
    close_modal("#choose_new_round_modal");
    show_grand_final_modal();
}

function show_grand_final_modal() {
    $(".multi_den_only").removeClass("hidden");
    $(".single_den_only").addClass("hidden");
    // TODO $("#new_round_modal").addClass("wide_modal");
    // Have to suspend updates to this dialog while it's open
    g_new_round_modal_open = true;
    show_modal("#new_round_modal", function(event) {
        handle_grand_final_submit();
        return false;
    });
}

function handle_grand_final_submit() {
    g_new_round_modal_open = false;
    close_modal("#new_round_modal");
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

function handle_master_next_to_race() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'select-heat',
                   heat: 'first',
                   now_racing: 0},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

// Parsing for coordinator-poll output

/* <current-heat now-racing= use-master-sched= classid= roundid= round=
                 group= heat= /> */
function parse_current_heat(data) {
    var current_xml = data.getElementsByTagName("current-heat")[0];
    if (!current_xml) {
        return false;
    }
    // NOTE: heats_scheduled gets written in process_coordinator_poll_response
    return {roundid: current_xml.getAttribute('roundid'),
            heat: current_xml.getAttribute('heat'),
            is_racing: current_xml.getAttribute('now-racing') == '1',
            master_schedule: current_xml.getAttribute('use-master-sched') == '1'};
}

function parse_rounds(data) {
    var rounds_xml = data.getElementsByTagName("round");
    var rounds = new Array(rounds_xml.length);
    for (var i = 0; i < rounds_xml.length; ++i) {
        var round_xml = rounds_xml[i];
        rounds[i] = {roundid: 1*round_xml.getAttribute('roundid'),
                     classid: 1*round_xml.getAttribute('classid'),
                     classname: round_xml.getAttribute('class'),
                     round: 1*round_xml.getAttribute('round'),
                     roster_size: 1*round_xml.getAttribute('roster_size'),
                     racers_passed: 1*round_xml.getAttribute('passed'),
                     racers_unscheduled: 1*round_xml.getAttribute('unscheduled'),
                     racers_scheduled: round_xml.getAttribute('passed') - round_xml.getAttribute('unscheduled'),
                     heats_scheduled: 1*round_xml.getAttribute('heats_scheduled'),
                     heats_run: 1*round_xml.getAttribute('heats_run'),
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
                lanes: tstate_xml.getAttribute('lanes')};
    }
}

function parse_replay_state(data) {
    var replay_state_xml = data.getElementsByTagName("replay-state")[0];
    return {status: replay_state_xml.textContent,
            icon: replay_state_xml.getAttribute('icon'),
            connected: replay_state_xml.getAttribute('connected')};
}

function parse_kiosk_pages(data) {
    var kiosk_pages_xml = data.getElementsByTagName("kiosk-page");
    var kiosk_pages = new Array(kiosk_pages_xml.length);
    for (var i = 0; i < kiosk_pages_xml.length; ++i) {
        kiosk_pages[i] = {brief: kiosk_pages_xml[i].getAttribute('brief'),
                          path: kiosk_pages_xml[i].textContent};
    }
    return kiosk_pages;
}

function parse_kiosks(data) {
    var kiosks_xml = data.getElementsByTagName("kiosk");
    var kiosks = new Array(kiosks_xml.length);
    for (var i = 0; i < kiosks_xml.length; ++i) {
        var kiosk_xml = kiosks_xml[i];
        kiosks[i] = {name: kiosk_xml.getElementsByTagName("name")[0].textContent,
                     address: kiosk_xml.getElementsByTagName("address")[0].textContent,
                     last_contact: kiosk_xml.getElementsByTagName("last_contact")[0].textContent,
                     assigned_page: kiosk_xml.getElementsByTagName("assigned_page")[0].textContent};
    }
    return kiosks;
}

function parse_racers(data) {
    var racers_xml = data.getElementsByTagName("racer");
    var racers = new Array(racers_xml.length);
    for (var i = 0; i < racers_xml.length; ++i) {
        racers[i] = {lane: racers_xml[i].getAttribute("lane"),
                     name: racers_xml[i].getAttribute("name"),
                     carnumber: racers_xml[i].getAttribute("carnumber"),
                     finishtime: racers_xml[i].getAttribute("finishtime")};
    }
    return racers;
}

// Generate page contents in response to coordinator-poll output

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

function generate_kiosk_control_group(index, kiosk, pages) {
    var div = $("<div class=\"block_buttons\"/>");
    var elt = $("<div class=\"control_group kiosk_control\"/>");

    elt.append("<p>Kiosk <span class=\"kiosk_control_name\">" + kiosk.name + "</span>"
               + " <span class=\"kiosk_control_address\">" + kiosk.address + "</span>"
               + "</p>");
    elt.append("<p class=\"last_contact\">Last contact: " + kiosk.last_contact + "</p>");
    elt.append("<label for=\"kiosk-page-" + index + "\">Display:</label>");
    var sel = $("<select name=\"kiosk-page-" + index + "\"" 
                + " data-kiosk-address=\"" + kiosk.address + "\"" 
                + " onchange=\"handle_assign_kiosk_page_change(this)\""
                + "/>");
    for (var i = 0; i < pages.length; ++i) {
        opt = $("<option value=\"" + pages[i].path + "\">" + pages[i].brief + "</option>");
        if (kiosk.assigned_page == pages[i].path) {
            opt.prop("selected", true);
        }
        sel.append(opt);
    }

    sel.appendTo(elt);
    elt.append('<input type="button" data-enhanced="true"'
               + ' onclick="show_kiosk_naming_modal(\'' + kiosk.address + '\', \'' + kiosk.name + '\')"'
               + ' value="Assign Name"/>');

    elt.appendTo(div);
    div.appendTo("#kiosk_control_group");
}

function emit_progress_bars(elt) {
    elt.append("<div class='racers progress'>"
               + "<div class='bar2'>"
               + "<div class='bar1'></div>"
               + "</div>"
               + "</div>");
    elt.append("<div class='heats progress'>"
               + "<div class='bar1'></div>"
               + "</div>");
}

function inject_progress_bars(group, round) {
    if (round.roster_size > 0) {
        // bar2 (yellow) = passed
        // bar1 (blue) = scheduled
        // Trouble here will be scheduled-but-no-longer-passed, i.e., scheduled > passed
        var passed = Math.max(round.racers_passed, round.racers_scheduled);
        group.find(".racers .bar2").width((100 * passed / round.roster_size) + '%');
        if (passed > 0) {
            group.find(".racers .bar1").width((100 * round.racers_scheduled / passed) + '%');
        }
    }

    if (round.heats_scheduled > 0) {
        group.find(".heats .bar1").width((100 * round.heats_run / round.heats_scheduled) + '%');
    }
}

// collapsible is a string containing space-separated css class names, possibly empty.
function emit_progress_text(elt, collapsible) {
    elt.append('<div class="' + collapsible + '">'
               + '<p><span data-name="roster_size"></span> racer(s), '
               + '<span data-name="n_passed"></span> passed, ' 
               + '<span data-name="scheduled"></span> in schedule.<br/>'
               + '<span data-name="n_heats_scheduled"></span> heats scheduled, '
               + '<span data-name="n_heats_run"></span> run.</p>'
               + '</div>');
}

function inject_progress_text(group, round) {
    group.find("[data-name=roster_size]").text(round.roster_size);
    group.find("[data-name=n_passed]").text(round.racers_passed);
    group.find("[data-name=scheduled]").text(round.racers_scheduled);
    group.find("[data-name=n_heats_scheduled]").text(round.heats_scheduled);
    group.find("[data-name=n_heats_run]").text(round.heats_run);
}

function generate_scheduling_control_group(round, current) {
    var elt = $("<div data-roundid=\"" + round.roundid + "\" class=\"control_group scheduling_control\"/>");
    var collapsible = " collapsible";
    if (round.roundid == current.roundid) {
        elt.addClass('current');
        collapsible = "";
    }

    elt.append('<div class="roundno">' + round.round + '</div>');
    elt.append('<h3>' + (round.roundid == current.roundid ? '' :
                         '<img data-name="triangle" src="img/triangle_east.png"/>')
                      + round.classname + '</h3>');

    emit_progress_text(elt, collapsible);
    emit_progress_bars(elt);

    // Which buttons appear depends on a bunch of the parameters.
    // It should be OK for inject to just rewrite the buttons every time.
    var buttons = $("<div data-name=\"buttons\" class=\"block_buttons" + collapsible + "\"/>");
    buttons.appendTo(elt);

    if (round.roundid != current.roundid) {
        elt.find(".collapsible").hide();
        elt.click(function() {
            var closed = elt.find(".collapsible").css("display") == "none";
            // Don't want the current round to collapse like this...

            $("img[data-name=triangle]").attr('src', 'img/triangle_east.png');
            $(".collapsible").slideUp(200);

            if (closed) {
                elt.find("img[data-name=triangle]").attr('src', 'img/triangle_south.png');
                elt.find(".collapsible").slideDown(200); 
            }
        });
    }

    // By this rule, changes to n_heats_run and n_heats_scheduled and
    // current.roundid would change the order for the rounds.
    elt.appendTo("#" + round.category + "-group");

    inject_into_scheduling_control_group(round, current);
}

// Injects new values into an existing scheduling control group.  The
// available control buttons get rewritten entirely.
function inject_into_scheduling_control_group(round, current) {
    var group = $("[data-roundid=" + round.roundid + "]");

    inject_progress_text(group, round);
    inject_progress_bars(group, round);

    var buttons = group.find("[data-name=buttons]");
    buttons.empty();

    if (round.racers_unscheduled > 0) {
        if (round.heats_run == 0) {
            buttons.append('<input type="button" data-enhanced="true"'
                           + ' onclick="show_schedule_modal(' + round.roundid + ')"'
                           + ' value="Schedule"/>');
        } else if (false /* TODO: Reschedule is not ready for prime time */) {
            buttons.append('<input type="button" data-enhanced="true"' 
                           + ' onclick="handle_reschedule_button(' + round.roundid + ')"'
                           + ' value="Reschedule"/>');
        }
    }

    if (round.roundid != current.roundid) {
        // TODO: Don't offer 'race' choice for single roundid under master scheduling
        if (round.heats_run > 0) {
            buttons.append('<input type="button" data-enhanced="true"'
                           + ' onclick="handle_make_changes_button(' + round.roundid + ')"'
                           + ' value="Make Changes"/>');
        } else if (round.heats_scheduled > 0 && round.heats_run < round.heats_scheduled) {
            buttons.append('<input type="button" data-enhanced="true"'
                           + ' onclick="handle_race_button(' + round.roundid + ')"'
                           + ' value="Race"/>');
        }
    }

    // TODO: AND there isn't already a next round or grand finals with
    // finishtimes...
    // TODO: Un-generate a round?  GPRM allows deleting rounds, but
    // apparently not the first round.
    if (false && round.heats_scheduled > 0 && round.heats_run == round.heats_scheduled) {
        buttons.append('<input type="button" data-enhanced="true"'
                       + ' onclick="show_new_round_modal(' + round.roundid + ')"'
                       + ' value="New Round"/>');
    }
}

function generate_kiosk_controls(pages, kiosks) {

    var hash = 0;
    for (var i = 0; i < kiosks.length; ++i) {
        var kiosk = kiosks[i];
        hash = hash_string(hash, kiosk.name);
        hash = hash_string(hash, kiosk.address);
        hash = hash_string(hash, kiosk.last_contact);
        hash = hash_string(hash, kiosk.assigned_page);
    }
    if (hash != g_kiosk_hash) {
        $("#kiosk_control_group").empty();
        for (var i = 0; i < kiosks.length; ++i) {
            generate_kiosk_control_group(i, kiosks[i], pages);
        }
        g_kiosk_hash = hash;
    }
}

function generate_current_heat_racers(racers, current) {
    g_current_heat_racers = racers;
    // TODO: Assumes now-racing-group empty?
    if (racers.length > 0) {
        $("#now-racing-group").prepend("<div class='heat-lineup'><table>" 
                                       + "<tr>" 
                                       + "<th>Lane</th>"
                                       + "<th>Racer</th>"
                                       + "<th>Car</th>"
                                       + "<th>Time</th>" 
                                       + "</tr>"
                                       + "</table>"
                                       + "<h3>Heat " + current.heat + " of "
                                           + current.heats_scheduled + "</h3>"
                                       + "</div>");

        var racers_table = $("#now-racing-group table");
        for (var i = 0; i < racers.length; ++i) {
            racers_table.append('<tr><td>' + racers[i].lane + '</td>'
                                + '<td>' + racers[i].name + '</td>'
                                + '<td>' + racers[i].carnumber + '</td>'
                                + '<td>' + racers[i].finishtime + '</td>'
                                + '</tr>');
        }
    }
}

function calculate_totals(rounds) {
    var max_round = -1;
    var total_roster_size = 0;
    var total_racers_passed = 0;
    var total_racers_scheduled = 0;
    var total_heats_scheduled = 0;
    var total_heats_run = 0;
    for (var i = 0; i < rounds.length; ++i) {
        var round = rounds[i];
        console.log(round);
        max_round = Math.max(max_round, round.round);
        total_roster_size += round.roster_size;
        total_racers_passed += round.racers_passed;
        total_racers_scheduled += round.racers_scheduled;
        total_heats_scheduled += round.heats_scheduled;
        total_heats_run += round.heats_run;
    }
    return {round: max_round,
            roster_size: total_roster_size,
            racers_passed: total_racers_passed,
            racers_scheduled: total_racers_scheduled,
            heats_scheduled: total_heats_scheduled,
            heats_run: total_heats_run};
}

function offer_new_rounds(rounds) {
    if (g_new_round_modal_open) {
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
            completed_rounds.push(round);
        }
    }

    if (completed_rounds.length > 0) {
        var add_grand_finals = completed_rounds.length > 1;
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
                    button.on('click', function(event) { handle_new_round_chosen(round.roundid); });
                    modal.append(button);

                    var label = $('<label for="roundid_' + round.roundid + '"></label>');
                    label.text(round.classname);
                    multi_flipswitches.append(label);
                    multi_flipswitches.append('<input type="checkbox"'
                                              + ' data-role="flipswitch"'
                                              + ' id="roundid_' + round.roundid + '"' 
                                              + ' name="roundid_' + round.roundid + '"' 
                                              + ' checked="checked"/>');

                    completed_rounds.splice(i, 1);
                } else {
                    ++i;
                }
            }
        }
        if (add_grand_finals) {
            modal.append('<h3>Grand Finals</h3>');
            var button = $('<input type="button" data-enhanced="true" value="Grand Finals"/>');
            button.on('click', function(event) { handle_grand_final_chosen(); });
            modal.append(button);
            multi_flipswitches.trigger("create");
        }
        modal.append('<h3>&nbsp;</h3>');
        modal.append('<input type="button" data-enhanced="true" value="Cancel"'
                     + ' onclick=\'close_modal("#choose_new_round_modal");\'/>');
        // Show the block with the "Add New Rounds" button
        $("#add_new_rounds_group").removeClass("hidden");
    } else {
        // Hide the block with the "Add New Rounds" button
        $("#new_round_controls").addClass("hidden");
    }
}

function process_coordinator_poll_response(data) {
    var current = parse_current_heat(data);
    if (!current) {
        console.log("Returning early because no current heat");
        return;
    }
    update_for_current_round(current);

    var rounds = parse_rounds(data);

    offer_new_rounds(rounds);

    $("#master-schedule-group").empty();
    if (current.master_schedule) {  // TODO: Consolidate with generate_scheduling_round_group?
        var totals = calculate_totals(rounds);

        var elt = $("<div class=\"control_group scheduling_control\"/>");
        var collapsible = "";

        elt.append('<div class="roundno">' + totals.round + '</div>');
        elt.append('<h3>Master Schedule</h3>');

        emit_progress_text(elt, collapsible);
        emit_progress_bars(elt);

        var buttons = $("<div data-name=\"buttons\" class=\"block_buttons" + collapsible + "\"/>");
        buttons.appendTo(elt);
        buttons.append('<input type="button" data-enhanced="true"'
                       + ' onclick="handle_master_next_to_race()" value="Next To Race"/>');
        elt.appendTo("#master-schedule-group");

        inject_progress_text(elt, totals);
        inject_progress_bars(elt, totals);
    }

    var layout = {'now-racing': [],
                  'ready-to-race': [],
                  'done-racing': [],
                  'not-yet-scheduled': []};
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
        $("#now-racing-group .heat-lineup").remove();
        $.each(rounds, function (index, round) {
            inject_into_scheduling_control_group(round, current);
        });
    } else {
        g_rounds_layout = layout;
        $(".scheduling_control_group").empty();
        $.each(rounds, function (index, round) {
            generate_scheduling_control_group(round, current);
        });
    }

    generate_timer_state_group(parse_timer_state(data));

    generate_replay_state_group(parse_replay_state(data));

    generate_kiosk_controls(parse_kiosk_pages(data), parse_kiosks(data));

    generate_current_heat_racers(parse_racers(data), current);

    $("#kiosk_control_group").trigger("create");
}

function coordinator_poll() {
    $.ajax(g_action_url,
           {type: 'GET',
            data: {query: 'coordinator-poll'},
            success: function(data) {
                setTimeout(coordinator_poll, /* TODO: 2000 */ 6000);
                process_coordinator_poll_response(data);
                //$(".collapsible").slideUp();
                //setTimeout(function() { $(".collapsible").slideDown(); }, 2000);
            },
            error: function() {
                setTimeout(coordinator_poll, 2000);
            }
});
}


$(function() { coordinator_poll(); });
