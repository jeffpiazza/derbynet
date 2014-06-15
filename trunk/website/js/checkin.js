// Handlers for the UI controls in the check-in page.
// TODO: Handle permission problems reported for the action.

g_checkin_action_url = "action.php";

var pending_ajax_requests = 0;

function ajax_add_request() {
  pending_ajax_requests += 1;
  $('#ajax_num_requests').html(pending_ajax_requests);
  $('#ajax_working').removeClass('hidden');
}

function ajax_remove_request() {
  pending_ajax_requests -= 1;
  $('#ajax_num_requests').html(pending_ajax_requests);
  if (pending_ajax_requests == 0)
	$('#ajax_working').addClass('hidden');
}

$(document).ajaxSend(function(event, xhr, options) {
    ajax_add_request();
});

$(document).ajaxComplete(function(event, xhr, options) {
    ajax_remove_request();
});

$(document).ajaxSuccess(function(event, xhr, options, xmldoc) {
	var fail = xmldoc.documentElement.getElementsByTagName("failure");

	if (fail && fail.length > 0) {
		alert("Action failed: " + fail[0].textContent);
	}
});

$(document).ajaxSuccess(function(event, xhr, options, xmldoc) {
	var reload = xmldoc.documentElement.getElementsByTagName("reload");
	if (reload && reload.length > 0) {
        console.log('ajaxSuccess event: reloading page');
		location.reload(true);
	}
});

$(document).ajaxSuccess(function(event, xhr, options, xmldoc) {
	var passed = xmldoc.documentElement.getElementsByTagName("passed");
    if (passed && passed.length > 0) {
		var racerid = passed[0].getAttribute("racer");
		$("#passed-" + racerid).prop('checked', true);
	}
});

// This executes when a checkbox for "Passed" is clicked.
function handlechange_passed(cb, racer) {
    // cb is the checkbox element, with name "pased-" plus the racer id, e.g., passed-1234
    if (!cb.checked && !confirm("Are you sure you want to unregister " + racer + "?")) {
	    cb.checked = true;
	    return;
    }
    // 7 = length of "passed-" prefix
    var racer = cb.name.substring(7);
    var value = cb.checked ? 1 : 0;

    $.ajax(g_checkin_action_url,
           {type: 'POST',
            data: {action: 'pass',
                   racer: racer,
                   value: value},
           });
}

// This executes when a checkbox for "Exclusively by Scout" is clicked.
function handlechange_xbs(cb) {
    // cb is the checkbox element, with name "xbs-" plus the racer id, e.g., xbs-1234
    // 4 = length of "xbs-" prefix
    var racer = cb.name.substring(4);
    var value = cb.checked ? 1 : 0;

    $.ajax(g_checkin_action_url,
           {type: 'POST',
            data: {action: 'xbs',
                   racer: racer,
                   value: value},
           });
}

// This executes when a car's number cell is clicked; we present an
// overlaid form to renumber the car.
function show_renumber_form(racer_name, racerid, td) {
  $("#renumber_racerprompt").html(racer_name);
  $("#renumber_racer").val(racerid);
  $("#renumber_carno").val(td.innerHTML);
  $("#renumberform").removeClass("hidden");
  $("#renumber_carno").focus();
}

// This runs when the submit button for the car-renumbering form is
// clicked.
function handle_renumber() {
    var racerid = $("#renumber_racer").val();
    var new_carno = $("#renumber_carno").val();
    $.ajax(g_checkin_action_url,
           {type: 'POST',
             data: {action: 'renumber',
                   racer: racerid,
                   value: new_carno},
            success: function(data) {
                $("#renumber" + racerid).html(new_carno);
            },
           });
   $("#renumberform").addClass("hidden");
}


function show_rank_change_form(racer_name, racerid, rankid, td) {
    var rank_picker = $("#rank_picker");

    $("#rank_racerprompt").html(racer_name);
    $("#rank_racer").val(racerid);

    // $("option", rank_picker).removeProp("selected");
    // $("[value=" + rankid + "]", rank_picker).prop("selected", true);
    rank_picker.val(rankid);
    rank_picker.change();

    $("#rankchangeform").removeClass("hidden");
    rank_picker.focus();
}

function handle_rank_change() {
    var racerid = $("#rank_racer").val();
    var rank_picker = $("#rank_picker");
    var value = rank_picker.val();
    var option = $('[value="' + value + '"]', rank_picker);
    var classname = option.attr('data-class');
    var rankname = option.attr('rank-class');

    $.ajax(g_checkin_action_url,
           {type: 'POST',
             data: {action: 'classchange',
                   racer: racerid,
                   value: value},
            success: function(data) {
                $('#class-' + racerid).text(classname);
                $('#rank-' + racerid).text(rankname);
            },
           });

   $("#rankchangeform").addClass("hidden");
}
