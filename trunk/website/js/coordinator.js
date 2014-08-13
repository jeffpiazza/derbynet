
// TODO: This preamble copied from checkin.js; refactor!

g_action_url = "action.php";

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

g_current_heat_racers = new Array();

// Processing a response from coordinator_poll can force the isracing
// flipswitch to change state.  We don't want that change to trigger
// an ajax request to update the database: if we get a bit out of sync
// with the database, we'll get into a feedback cycle that never ends.
//
// Use this boolean to mark changes induced programmatically, as
// opposed to user input.
g_updating_current_round = false;

// Controls for current racing group:

function handle_isracing_change(event, scripted) {
    console.log('handle_isracing_change: checked=' + $("#is-currently-racing").prop('checked')
                + ', scripted=' + scripted
                + ', g_updating=' + g_updating_current_round);  // TODO
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
    modal_div.find("#kiosk_name_field").val(name);
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

function show_kiosk_naming_modal(address, name) {
    show_modal("#kiosk_modal", function(event) {
        handle_name_kiosk(address, $("#kiosk_name_field").val());
        return false;
    });
}

function handle_name_kiosk(address, name) {
    close_kiosk_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'assign-kiosk',
                   address: address,
                   name: name},
           });
}

function close_kiosk_modal() {
    $("#modal_background").fadeOut(200);
    $("#kiosk_modal").addClass("hidden");
    $("#kiosk_modal").css({'display': 'none'});
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
    close_manual_results_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data: $("#manual_results_modal form").serialize(),
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function handle_discard_results_button() {
    close_manual_results_modal();
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

function close_manual_results_modal() {
    $("#modal_background").fadeOut(200);
    $("#manual_results_modal").addClass("hidden");
    $("#manual_results_modal").css({'display': 'none'});
}

// Controls for racing rounds

function show_schedule_modal(roundid) {
    show_modal("#schedule_modal", function(event) {
        handle_schedule_submit(roundid, $("#schedule_num_rounds").val());
        return false;
    });
}

function handle_schedule_submit(roundid, rounds) {
    close_schedule_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'schedule',
                   roundid: roundid,
                   nrounds: rounds},
            success: function(data) { process_coordinator_poll_response(data); }
           });
}

function close_schedule_modal() {
    $("#modal_background").fadeOut(200);
    $("#schedule_modal").addClass("hidden");
    $("#schedule_modal").css({'display': 'none'});
}

function handle_reschedule_button(roundid) {
    // TODO: On success
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


// Generate page contents in response to coordinator-poll output

/* <current-heat now-racing= use-master-sched= classid= roundid= round=
                 group= heat= /> */

function update_for_current_round(current_heat) {
    var isracing_checkbox = $("#is-currently-racing");
    var is_racing = (current_heat.getAttribute('now-racing') == '1');

    if (isracing_checkbox.prop('checked') != is_racing) {
        console.log('update_for_current_round: isracing_checkbox.prop(\'checked\')=' + isracing_checkbox.prop('checked')
+ '; is_racing=' + is_racing); // TODO
        isracing_checkbox.prop('checked', is_racing);
        // isracing_checkbox.change(); TODO
        g_updating_current_round = true;
        try {
            console.log('update_for_current_round: triggering scripted change event');
            isracing_checkbox.trigger("change", true);
            // Are the events handled asynchronously?
        } finally {
            g_updating_current_round = false;
        }
    }
}

function generate_timer_state_group(tstate) {
    $("#timer_status_text").text(tstate.textContent);
    $("#timer_status_icon").attr('src', tstate.getAttribute('icon'));
    var lanes = tstate.getAttribute('lanes');
    if (lanes != '') {
        $("#lane_count").text(lanes);
    }
}

function generate_replay_state_group(replay_state) {
    $("#replay_status").text(replay_state.textContent);
    $("#replay_status_icon").attr('src', replay_state.getAttribute('icon'));
    if (replay_state.getAttribute('host-and-port') == '') {
        $("#test_replay").addClass("hidden");
    } else {
        $("#test_replay").removeClass("hidden");
    }
}

function generate_kiosk_control_group(index, name, address, last_contact, assigned_page, pages) {
    var div = $("<div class=\"block_buttons\"/>");
    var elt = $("<div class=\"control_group kiosk_control\"/>");
    // TODO: Allow naming of the kiosks; otherwise it's just IP addresses
    elt.append("<p>Kiosk <span class=\"kiosk_control_name\">" + name + "</span>"
               + " <span class=\"kiosk_control_address\">" + address + "</span>"
               + "</p>");
    elt.append("<p class=\"last_contact\">Last contact: " + last_contact + "</p>");
    elt.append("<label for=\"kiosk-page-" + index + "\">Display:</label>");
    var sel = $("<select name=\"kiosk-page-" + index + "\"" 
                + " data-kiosk-address=\"" + address + "\"" 
                + " onchange=\"handle_assign_kiosk_page_change(this)\""
                + "/>");
    for (var i = 0; i < pages.length; ++i) {
        opt = $("<option value=\"" + pages[i].path + "\">" + pages[i].brief + "</option>");
        if (assigned_page == pages[i].path) {
            opt.prop("selected", true);
        }
        sel.append(opt);
    }

    sel.appendTo(elt);
    elt.append('<input type="button" data-enhanced="true"'
               + ' onclick="show_kiosk_naming_modal(\'' + address + '\', \'' + name + '\')"'
               + ' value="Assign Name"/>');

    elt.appendTo(div);
    div.appendTo("#kiosk_control_group");
}

function generate_scheduling_control_group(roundid, round_class, round, roster_size, n_passed, n_unscheduled, 
                                           n_heats_scheduled, n_heats_run, current) {
    var elt = $("<div class=\"control_group scheduling_control\"/>");
    if (roundid == current.roundid) {
        elt.addClass('current');
    }

    elt.append('<h3>' + round_class + ', round ' + round
               + (roundid == current.roundid ? '; heat ' + current.heat + ' of ' + n_heats_scheduled : '')
               + '</h3>');
    elt.append('<p>' + roster_size + ' racer(s), ' + n_passed + ' passed, ' 
               + (n_passed - n_unscheduled) + ' in schedule.</p>');
    elt.append('<p>' + n_heats_scheduled + ' heats scheduled, ' + n_heats_run + ' run.</p>');

    var buttons = $("<div class=\"block_buttons\"/>");
    buttons.appendTo(elt);

    if (n_unscheduled > 0) {
        if (n_heats_run == 0) {
            buttons.append('<input type="button" data-enhanced="true"'
                           + ' onclick="show_schedule_modal(' + roundid + ')"'
                           + ' value="Schedule"/>');
        } else {
            buttons.append('<input type="button" data-enhanced="true"' 
                           + ' onclick="handle_reschedule_button(' + roundid + ')"'
                           + ' value="Reschedule"/>');
        }
    }

    if (roundid != current.roundid) {
        // TODO: Don't offer 'race' choice for single roundid under master scheduling
        if (n_heats_run > 0) {
            buttons.append('<input type="button" data-enhanced="true"'
                           + ' onclick="handle_make_changes_button(' + roundid + ')"'
                           + ' value="Make Changes"/>');
        } else if (n_heats_scheduled > 0 && n_heats_run < n_heats_scheduled) {
            buttons.append('<input type="button" data-enhanced="true"'
                           + ' onclick="handle_race_button(' + roundid + ')"'
                           + ' value="Race"/>');
        }
    }
    
    elt.appendTo(roundid == current.roundid ? "#now-racing-group"
                 : n_heats_run < n_heats_scheduled ? "#ready-to-race-group"
                 : n_heats_run > 0 ? "#done-racing-group"
                 : "#not-yet-scheduled-group");
}

function process_coordinator_poll_response(data) {
    $(".scheduling_control_group").empty();
    var current_heat = data.getElementsByTagName("current-heat")[0];
    var current = {roundid: current_heat.getAttribute('roundid'),
                   heat: current_heat.getAttribute('heat')};
    var rounds = data.getElementsByTagName("round");

    update_for_current_round(current_heat);
    for (var i = 0; i < rounds.length; ++i) {
        var round = rounds[i];
        generate_scheduling_control_group(
            round.getAttribute('roundid'),
            round.getAttribute('class'),
            round.getAttribute('round'),
            round.getAttribute('roster_size'),
            round.getAttribute('passed'),
            round.getAttribute('unscheduled'),
            round.getAttribute('heats_scheduled'),
            round.getAttribute('heats_run'),
            current);
    }

    var timer_state = data.getElementsByTagName("timer-state")[0];
    generate_timer_state_group(timer_state);

    var replay_state = data.getElementsByTagName("replay-state")[0];
    generate_replay_state_group(replay_state);

    var kiosk_pages = data.getElementsByTagName("kiosk-page");
    var pages = new Array(kiosk_pages.length);
    for (var i = 0; i < kiosk_pages.length; ++i) {
        pages[i] = {brief: kiosk_pages[i].getAttribute('brief'),
                    path: kiosk_pages[i].textContent};
    }

    $("#kiosk_control_group").empty();
    var kiosks = data.getElementsByTagName("kiosk");
    for (var i = 0; i < kiosks.length; ++i) {
        var kiosk = kiosks[i];
        generate_kiosk_control_group(
            i,
            kiosk.getElementsByTagName("name")[0].textContent,
            kiosk.getElementsByTagName("address")[0].textContent,
            kiosk.getElementsByTagName("last_contact")[0].textContent,
            kiosk.getElementsByTagName("assigned_page")[0].textContent,
            pages);
    }

    var racer_elements = data.getElementsByTagName("racer");
    g_current_heat_racers = new Array(racer_elements.length);
    $("#now-racing-group").prepend("<table class='heat-lineup'>" 
                                   + "<tr>" 
                                   + "<th>Lane</th>"
                                   + "<th>Racer</th>"
                                   + "<th>Car</th>"
                                   + "<th>Time</th>" 
                                   + "</tr>"
                                   + "</table>");
    var racers_table = $("#now-racing-group table");
    for (var i = 0; i < racer_elements.length; ++i) {
        g_current_heat_racers[i] = {lane: racer_elements[i].getAttribute("lane"),
                                    name: racer_elements[i].getAttribute("name"),
                                    carnumber: racer_elements[i].getAttribute("carnumber"),
                                    finishtime: racer_elements[i].getAttribute("finishtime")};
        racers_table.append('<tr><td>' + racer_elements[i].getAttribute("lane") + '</td>'
                            + '<td>' + racer_elements[i].getAttribute("name") + '</td>'
                            + '<td>' + racer_elements[i].getAttribute("carnumber") + '</td>'
                            + '<td>' + racer_elements[i].getAttribute("finishtime") + '</td>'
                            + '</tr>');
    }

    $("#kiosk_control_group").trigger("create");
}

function coordinator_poll() {
    console.log("coordinator_poll");
    $.ajax(g_action_url,
           {type: 'GET',
            data: {query: 'coordinator-poll'},
            success: function(data) {
                setTimeout(coordinator_poll, 2000);
                process_coordinator_poll_response(data);
            },
            error: function() {
                setTimeout(coordinator_poll, 2000);
            }
});
}


$(function() { coordinator_poll(); });
