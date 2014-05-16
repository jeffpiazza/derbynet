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

function readystate_handler() {
    // "this" = XMLHttpRequest
    // attribute DOMString responseText
    // attribute Document responseXML
    // attribute unsigned short status
    // attribute DOMString statusText

   if (this.readyState == this.DONE) {
	   ajax_remove_request();
	   var respXML = this.responseXML;
	   if (!respXML) alert("No XML in response.  status = " + this.status
						   + " (" + this.statusText + ")");

	   var response = respXML.documentElement;
	   var fail = response.getElementsByTagName("failure");
	   var reload = response.getElementsByTagName("reload");
	   var passed = response.getElementsByTagName("passed");

	   if (fail && fail.length > 0) {
		 alert("Action failed: " + fail[0].textContent);
	   }

	   if (reload && reload.length > 0) {
		   location.reload(true);
       } else if (passed && passed.length > 0) {
		 var racerid = passed[0].getAttribute("racer");
		 $("#passed-" + racerid).prop('checked', true);
	   }
   }
}

// This executes when a checkbox for "Passed" is clicked.
function handlechange_passed(cb, racer) {
  // cb is the checkbox element, with name "pased-" plus the racer id, e.g., passed-1234
   if (!cb.checked && !confirm("Are you sure you want to unregister " + racer + "?")) {
	   cb.checked = true;
	   return;
   }

   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", g_checkin_action_url, /*async*/true);
   xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
   xmlhttp.onreadystatechange = readystate_handler;
   // 7 = length of "passed-" prefix
   ajax_add_request();
   xmlhttp.send("action=pass&racer=" + cb.name.substring(7) + "&value=" + (cb.checked ? 1 : 0));
}

// This executes when a checkbox for "Exclusively by Scout" is clicked.
function handlechange_xbs(cb) {
  // cb is the checkbox element, with name "xbs-" plus the racer id, e.g., passed-1234
   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", g_checkin_action_url, /*async*/true);
   xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
   xmlhttp.onreadystatechange = readystate_handler;
   // 4 = length of "xbs-" prefix
   xmlhttp.send("action=xbs&racer=" + cb.name.substring(4) + "&value=" + (cb.checked ? 1 : 0));
   ajax_add_request();
}

// This executes when a car's number cell is clicked; we present an overlaid form to renumber the car.
function show_renumber_form(racer_name, racerid, td) {
  $("#renumber_racerprompt").html(racer_name);
  $("#renumber_racer").val(racerid);
  $("#renumber_carno").val(td.innerHTML);
  $("#renumberform").removeClass("hidden");
  $("#renumber_carno").focus();
}

// This runs if the submit button for the car-renumbering form is clicked.
function handle_renumber() {
   var racerid = $("#renumber_racer").val();
   var new_carno = $("#renumber_carno").val();
   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", g_checkin_action_url, /*async*/true);
   xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
   xmlhttp.onreadystatechange = readystate_handler;
   xmlhttp.send("action=renumber&racer=" + racerid
		+ "&value=" + new_carno);
   ajax_add_request();

   $("#renumber" + racerid).html(new_carno);
   $("#renumberform").addClass("hidden");
}

function show_rank_change_form(racer_name, racerid, rankid, td) {
  $("#rank_racerprompt").html(racer_name);
  $("#rank_racer").val(racerid);

  $("option", "#rank_picker").removeAttr("selected");
  $("[value=" + rankid + "]", "#rank_picker").attr("selected", "1");

  $("#rankchangeform").removeClass("hidden");
  $("#rank_picker").focus();
}

function handle_rank_change() {
   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", g_checkin_action_url, /*async*/true);
   xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
   xmlhttp.onreadystatechange = readystate_handler;
   xmlhttp.send("action=classchange&racer=" + $("#rank_racer").val()
		+ "&value=" + $("#rank_picker").val());
   ajax_add_request();
   $("#rankchangeform").addClass("hidden");
}
