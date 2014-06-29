
function handle_login(role, pwd) {
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'login',
                   name: role,
                   password: pwd},
            success: function(/*PlainObject*/ response, /*String*/ textStatus, /*jqXHR*/ jqXHR) {
                response = $(response);
		        var succ = response.find("success");
		        if (succ && succ.size() > 0) {
			        window.location.href = 'index.php';
		        } else {
			        var fail = response.find("failure");
			        if (fail && fail.size() > 0) {
				        alert("Login fails: " + fail.text());
			        } else {
				        alert("Unrecognized XML: " + this.responseXML);
			        }
		        }
            },
           });
}

function handle_logout() {
    handle_login("", "");
}

function handle_password_submission(form) {
	var role = document.getElementById("name_for_password").value;
	var pwd = document.getElementById("pw_for_password").value;
	handle_login(role, pwd);
}

function show_password_form(name) {
	document.getElementById("name_for_password").value = name;
	$("#password_form").removeClass("hidden");
	// Giving immediate focus doesn't work, because the element is still invisible.
	// Waiting 100ms is clumsy, but reasonably effective.
	setTimeout(function() { 
			document.getElementById('pw_for_password').focus(); }, 100);
}
