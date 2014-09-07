
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

function show_choose_database_modal() {
  show_modal("#choose_database_modal", function(event) {
      handle_choose_database();
      return false;
  });
}

function handle_choose_database() {
    close_modal("#choose_database_modal");
    $.ajax('setup-action.php',
           {type: 'POST',
            data: $("#choose_database_modal form").serialize(),
            success: function(data) {
                console.log("Success");
                console.log(data);
	            var success = data.documentElement.getElementsByTagName("success");
                if (success && success.length > 0) {
                    alert("Configuration successful!");
                    location.reload(true);
                }
            }
           });
}

function show_initialize_schema_modal() {
  show_modal("#initialize_schema_modal", function(event) {
      handle_initialize_schema();
      return false;
  });
}

function handle_initialize_schema() {
    close_modal("#initialize_schema_modal");
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'run-sql',
                   script: 'schema'},
            success: function(data) {
	            var success = data.documentElement.getElementsByTagName("success");
                if (success && success.length > 0) {
                    alert("Schema definition successful!");
                    location.reload(true);
                }
            },
           });
}
