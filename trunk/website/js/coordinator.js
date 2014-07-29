
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

// End of preamble

function handle_test_replay() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'replay-test'}
           });
}

function handle_skip_heat() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'advance-heat',
                   heat: 'next'},
           });
}

function handle_previous_heat() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'advance-heat',
                   heat: 'prev'},
           });
}

// sel is the <select data-kiosk-address> input element
function handle_kiosk_page_change(sel) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'assign-kiosk',
                   address: sel.getAttribute('data-kiosk-address'),
                   page: sel.value},
           });
}

// TODO: no way to specify multiple heats per racer per lane (number of "rounds" in schedule)
function handle_schedule(roundid) {
    // TODO: On success, 
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'schedule',
                   roundid: roundid}});
}


function generate_scheduling_control_group(roundid, round_class, round, roster_size, n_passed, n_unscheduled, 
                                           n_heats_scheduled, n_heats_run, current_roundid) {
    var elt = $("<div class=\"control_group scheduling_control\"/>");
    if (roundid == current_roundid) {
        elt.addClass('current');
    }

    elt.append('<h3>' + round_class + ', round ' + round + '</h3>');
    elt.append('<p>' + roster_size + ' racer(s), ' + n_passed + ' passed, ' 
               + (n_passed - n_unscheduled) + ' scheduled.</p>');
    elt.append('<p>' + n_heats_scheduled + ' heats scheduled, ' + n_heats_run + ' run.</p>');

    var buttons = $("<div class=\"block_buttons\"/>");
    buttons.appendTo(elt);

    if (n_unscheduled > 0) {
        if (n_heats_scheduled == 0) {
            buttons.append('<input type="button" data-enhanced="true"'
                       + ' onclick="handle_schedule(' + roundid + ')"'
                       + ' value="Schedule"/>');
        } else {
            buttons.append('<input type="button" data-enhanced="true" value="Reschedule"/>');
        }
    }

    if (n_heats_run < n_heats_scheduled && roundid != current_roundid) {
        buttons.append('<input type="button" data-enhanced="true" value="Race"/>');
    }

    if (n_heats_run > 0) {
        buttons.append('<input type="button" data-enhanced="true" value="Discard Results"/>');
    }
    
    elt.appendTo("#scheduling-control-group");
}

function coordinator_poll() {
    console.log("coordinator_poll");
    $.ajax(g_action_url,
           {type: 'GET',
            data: {query: 'coordinator-poll'},
            success: function(data) {
                $("#scheduling-control-group").empty();
                var current_heat = data.getElementsByTagName("current-heat")[0];
                var rounds = data.getElementsByTagName("round");
                for (var i = 0; i < rounds.length; ++i) {
                    var round = rounds[i];
                    generate_scheduling_control_group(// TODO: round, roundid
                        round.getAttribute('roundid'),
                        round.getAttribute('class'),
                        round.getAttribute('round'),
                        round.getAttribute('roster_size'),
                        round.getAttribute('passed'),
                        round.getAttribute('unscheduled'),
                        round.getAttribute('heats_scheduled'),
                        round.getAttribute('heats_run'),
                        current_heat.getAttribute('roundid'));
                }
                setTimeout(coordinator_poll, 2000);
            },
            error: function() {
                setTimeout(coordinator_poll, 2000);
            }
});
}


$(function() { coordinator_poll(); });
