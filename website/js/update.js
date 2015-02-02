// -*- mode: javascript; js-indent-level: 2; c-basic-offset: 2 -*-

// Assumes ajax-failure.inc has already established a global ajax error handler

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


// Process <update resultid="nnn" time="n.nnn"/> elements
function process_update_elements(updates) {
  for (var i = 0; i < updates.length; ++i) {
	var upd = updates[i];
	var time_span = $(".resultid_" + upd.getAttribute("resultid")
	                  + " .time");
    if (time_span.length == 0) {
      console.log("Can't find time span for resultid " + upd.getAttribute("resultid"));
    }
    time_span.html(upd.getAttribute("time"));
  }
}

// Process <has_new_schedule roundid="..." groupid="..."/> elements.
// newly_scheduled is a NodeList, not an array.
// When showing results by schedule, it's the racing group that's
// relevant, otherwise it's the roundid
function process_new_schedules(newly_scheduled, index, completed) {
  if (index < newly_scheduled.length) {
    var nsched = newly_scheduled.item(index);
	var groupid = nsched.getAttribute(g_using_groupid ? "groupid" : "roundid");
    console.log("Loading page section for group_" + groupid);
	// The space in the load argument is significant!
	// The URL part presumably can't contain a space.

    // These assignments force curheat to get redone.
    // TODO: Visibility rule gets overridden
    g_cur_group = -1;
    g_cur_roundid = -1;
    g_cur_heat = -1;

    // This forces a refresh of all heat times, which may be necessary
    // if we're reloading a group that's already started racing.
    g_last_update_time = '';

	$("#group_" + groupid)
        .load(location.pathname + " #group_" + groupid + " tr",
              /* data */'',
              function() {
                console.log("Page load completed.");
                process_new_schedules(newly_scheduled, 1 + index, completed);
              });
  } else {
    completed();
  }
}

// Move curgroup class and add "Now Racing" text.
function notice_change_current_group(cgroup, cround, cclass) {
  // g_cur_group is initialized to -1, and query update-summary should keep
  // report group=-1 before racing starts.  While racing is actually
  // underway, group should be > 0.  When concluded, group should
  // report an empty string.
  if (cgroup != g_cur_group) {
	// Remove class and text from previous group, if any
	$(".curgroup").removeClass("curgroup");
	$(".pre_group_title").html("");

	// Mark the current racing group
	$(".group_" + cgroup).addClass("curgroup");
    var curgroup = $(".curgroup .pre_group_title");  // $(".group_" + cgroup + " .pre_group_title");
    if (curgroup.length > 0) {
	  curgroup.html("Now<br/>Racing");
    } else {
      console.log("Can't find title for new group " + cgroup);
    }

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
}

// Move curheat css class if it's changed
function notice_change_current_heat(croundid, heat) {
  // Scroll if necessary to see the heat AFTER current heat, but only if the
  // previously-current heat was visible.
  if (croundid != g_cur_roundid || heat != g_cur_heat) {
	var vis = true;
	var curheat = $(".curheat");
	if (curheat.length > 0)
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
}

function process_response_from_current(summary) {
  var current = summary.getElementsByTagName("current-heat")[0];
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

  var high_water = summary.getElementsByTagName("high_water")[0];

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

  g_last_update_time = high_water.getAttribute("completed");
  g_high_water_group = new_high_water_group;
  g_high_water_resultid = high_water.getAttribute("resultid");

  process_new_schedules(summary.getElementsByTagName("has_new_schedule"),
                        0,
                        function () {
                          notice_change_current_group(cgroup, cround, cclass);
                          notice_change_current_heat(croundid, heat);

                          process_update_elements(summary.getElementsByTagName("update"));

                          setTimeout(updatecurrent_fire, update_period);
                        });
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
  xmlhttp.open("GET", "action.php?query=update-summary&since=" + encodeURIComponent(g_last_update_time)
	             + "&high_water_resultid=" + g_high_water_resultid, /*async*/true);
  xmlhttp.onreadystatechange = updatecurrent_handler;
  xmlhttp.send("");
}

// This handler function processes the arriving XML document from update-summary
function updatecurrent_handler() {
  if (this.readyState == this.DONE) {
	if (this.status == 200) {
	  if (this.responseXML != null) {
	    cancel_ajax_failure();
	    process_response_from_current(this.responseXML.documentElement);
	  } else {
        // If the text returned from update-summary isn't parsable, e.g.,
        // because there's some kind of error on the php side, then
        // responseXML can come back null.  Rather than completely
        // freak out, let's try again in a moment.
		console.log("XmlHttpResponse:");
		console.log(this);
        ajax_failure(-1, "Response from server doesn't parse as XML.");
        setTimeout(updatecurrent_fire, 2500);  // 2.5 sec
      }
	} else {
      ajax_failure(this.status, this.statusText);
      setTimeout(updatecurrent_fire, 2500);  // 2.5 sec
 	}
  }
}

// TODO: Convert this poll to setInterval
$(document).ready(updatecurrent_fire);
