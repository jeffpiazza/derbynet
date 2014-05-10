// -*- mode: javascript; js-indent-level: 2; c-basic-offset: 2 -*-

// Updates the current page based on new data from the server.
// Relies on high-water values for resultid, roundid, and the 
// highest Completed timestamp for displayed heat times.

// The current round and heat get highlighted by moving the CSS classes
// curgroup and curheat.  These classes get removed from the old group/heat
// and applied to the new ones.  CSS then adds whatever visual effect
// we want (like different background color) to indicate the current heat.

var g_cur_group = -1;
var g_cur_roundid = -1;
var g_cur_heat = -1;

function process_response_from_current(summary) {
  var current = summary.getElementsByTagName("current_heat")[0];
  var croundid = current.getAttribute("roundid");
  var cround = current.getAttribute("round");
  var cgroup = g_using_groupid ? current.getAttribute("group") : croundid;
  var heat = current.getAttribute("heat");
  var cclass = current.firstChild ? current.firstChild.data : null;

  var nlanes = summary.getElementsByTagName("lanes")[0].getAttribute("n");

  var options = summary.getElementsByTagName("options")[0];
  var update_period = options.getAttribute("update-period");

  if (typeof g_use_master_sched != 'undefined' && 
	  g_use_master_sched != options.getAttribute("use-master-sched")) {
	console.log("Reload triggered by: g_use_master_sched=" + g_use_master_sched +
				"; use-master-sched option = " + options.getAttribute("use-master-sched"));
	location.reload(true);
	return;
  }

  if (typeof g_kiosk_page != 'undefined' && g_kiosk_page != "" && g_kiosk_page != options.getAttribute("kiosk-page")) {
	console.log("Reload triggered by: g_kiosk_page = " + g_kiosk_page +
				"; kiosk-page option = " + options.getAttribute("kiosk-page"));
	location.reload(true);
	return;
  }

  var high_water = summary.getElementsByTagName("high_water")[0];

  // Move curgroup class and add "Now Racing" text.

  // g_cur_group is initialized to -1, and current.php should keep
  // report group=-1 before racing starts.  While racing is actually
  // underway, group should be > 0.  When concluded, group should
  // report an empty string.
  if (cgroup != g_cur_group) {
	// Remove class and text from previous group, if any
	$(".curgroup").removeClass("curgroup");
	$(".pre_group_title").html("");

	// Mark the current racing group
	$(".group_" + cgroup).addClass("curgroup");
	$(".group_" + cgroup + " .pre_group_title").html("Now<br/>Racing");

	// Update text and target of "Now racing" link at top of page.
	if (cgroup) {
	  $(".now_running").html("Now racing: <a href='#group_" + cgroup + "'>"
							 + (cclass ? cclass + ", " : "")
							 + "Round " + cround + "</a>");
	} else {
	  $(".now_running").html("Racing has concluded (cgroup " + typeof cgroup + " " + cgroup 
							 + ", g_cur_group " + typeof g_cur_group + " " + g_cur_group + ".");
	}
	g_cur_heat = -2;
	g_cur_group = cgroup;
  }

  // Move curheat class if it's changed
  // Scroll if necessary to see the heat AFTER current heat, but only if the
  // previously-current heat was visible.
  if (croundid != g_cur_roundid || heat != g_cur_heat) {
	var vis = true;
	var curheat = $(".curheat");
	if (curheat[0])
	  vis = is_visible(curheat[0]);
	curheat.removeClass("curheat");
	curheat = $(".heat_" + croundid + "_" + heat);
	curheat.addClass("curheat");
	var nextheat = $(".heat_" + croundid + "_" + (parseInt(heat) + 1));
	if (!nextheat[0]) {
	  nextheat = curheat;
	}
	if (vis && nextheat[0])
	  setTimeout(function() { scroll_to_current(nextheat[0]); }, 250);

	g_cur_roundid = croundid;
	g_cur_heat = heat;
  }

  // Number of lanes may not have been known when the page was generated, but has since
  // been determined.  Update colspan attributes if that's the case.
  $(".wide").attr("colspan", nlanes);

  new_high_water_group = high_water.getAttribute(g_using_groupid ? "groupid" : "roundid")
    if (new_high_water_group > g_high_water_group) {
      // If we have new rounds set up, just reload the whole page
      console.log("Reload triggered by g_high_water_group=" + g_high_water_group);
      location.reload(true);
      return;
    }

  // <has_new_schedule roundid="..." groupid="..."/>
  // When showing results by schedule, it's the racing group that's relevant, otherwise it's the roundid
  var newly_scheduled = summary.getElementsByTagName("has_new_schedule");
  for (var i = 0; i < newly_scheduled.length; ++i) {
	var nsched = newly_scheduled[i].getAttribute(g_using_groupid ? "groupid" : "roundid");
	// The space in the load argument is significant!
	// The URL part presumably can't contain a space.
	$("#group_" + nsched).load(location.pathname + " #group_" + nsched + " tr");
	// TODO - Is this done synchronously or async?  If new content appears with
	// a subsequent <update> element, will the jscript below find the right (new) element?
	// Very rare to have a new schedule appear AND have rounds completed in it
	// faster than the page can update.
  }

  // Process <update resultid="nnn" time="n.nnn"/> elements
  var updates = summary.getElementsByTagName("update");
  for (var i = 0; i < updates.length; ++i) {
	var upd = updates[i];
	$(".resultid_" + upd.getAttribute("resultid")
	  + " .time").html(upd.getAttribute("time"));
  }

  g_last_update_time = high_water.getAttribute("completed");
  g_high_water_group = new_high_water_group;
  g_high_water_resultid = high_water.getAttribute("resultid");

  return update_period;
}

// True if any part of el is visible (vertically; doesn't check horizontally).
function is_visible(el) {
  var rect = el.getBoundingClientRect();
  return rect.bottom > 0 &&
         rect.top < $(window).height();
}


function scroll_to_current(el) {
  var rect = el.getBoundingClientRect();
  var w = $(window).height();

  if (rect.bottom > w) {  // Off-screen below.
    // $(window).stop().animate({scrollTop: $(window).scrollTop() + (rect.top + rect.bottom)/2 
    //	                                    - $(window).height()/2}, 800);
    $(window).scrollTop($(window).scrollTop() + (rect.top + rect.bottom)/2 - $(window).height()/2);
  }
}

function updatecurrent_fire() {
  var xmlhttp = new XMLHttpRequest();
  xmlhttp.open("GET", "current.php?since=" + encodeURIComponent(g_last_update_time)
	             + "&high_water_resultid=" + g_high_water_resultid, /*async*/true);
  xmlhttp.onreadystatechange = updatecurrent_handler;
  xmlhttp.send("");
}

// This handler function processes the arriving XML document from current.php.
function updatecurrent_handler() {
  if (this.readyState == this.DONE) {
	var update_period = 2500;  // 2.5 sec
	if (this.status == 200) {
	  $('#ajax_failure').addClass('hidden');
	  update_period = process_response_from_current(this.responseXML.documentElement);
	} else {
	  $('#ajax_status').html(this.status + " (" + 
							 (this.status == 0 ? "likely timeout" : this.statusText)
							 + ")");
	  $('#ajax_failure').removeClass('hidden');
	}

    // Repeat
    //console.log("Update period: " + update_period);
    setTimeout(updatecurrent_fire, update_period);
  }
}

$(document).ready(updatecurrent_fire);
