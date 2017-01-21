// Common ajax-handling functions for dashboard-like pages.  Looks for <reload/>
// elements to reload the page, and reports errors or failures via alerts.

g_action_url = "action.php";

// Note that this doesn't run if the $.ajax call has a 'success:' callback that
// generates an error.
$(document).ajaxSuccess(function(event, xhr, options, xmldoc) {
  var fail = xmldoc.documentElement.getElementsByTagName("failure");
  if (fail && fail.length > 0) {
    console.log(xmldoc);
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
  console.log(thrownError);
  console.log(jqXHR);
  console.log(jqXHR.responseText);
  console.log(event);
	alert("Ajax error: " + thrownError);
});

