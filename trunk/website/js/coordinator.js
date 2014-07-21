
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
    console.log('handle_kiosk_page_change');
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'assign-kiosk',
                   address: sel.getAttribute('data-kiosk-address'),
                   page: sel.value},
           });
}
