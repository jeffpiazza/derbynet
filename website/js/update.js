// Assumes ajax-failure.inc has already established a global ajax error handler

// Updates the current page based on new data from the server.
// Relies on high-water values for resultid, roundid, and the 
// highest Completed timestamp for displayed heat times.

// ondeck.php:
//    inc/rounds.inc: all_schedule_groups()
//  groupid is roundid for individual rounds, or just round number for master scheduling.
//
// <tbody id="tbody_{groupid}">
// <tr id="heat_{roundid}_{heat}">
// <td class="lane_{lane}" resultid="{resultid}">
//  ... <span class="time"></span> ...
//
// racer-results.php:
//   inc/rounds.inc: all_rounds(), groupid = roundid
// <tbody id="tbody_{groupid}">
// <tr class="d0">
// <td class="resultid_{resultid}">
// ... <span class="time"></span> ...

g_update_status.current = {tbodyid: -1,
                           // roundid and heat identify the current heat <tr>
                           // element, for ondeck.
                           roundid: -1,
                           heat: -1};

g_update_status.update_period = 2500;
g_update_status.interval_id = 0;

// Process <update resultid="nnn" time="n.nnn"/> elements; these are race times
// not previously recorded for entries already in the table.
function process_update_elements(updates) {
  for (var i = 0; i < updates.length; ++i) {
	var upd = updates[i];
	var time_span = $(".resultid_" + upd.getAttribute("resultid")
	                  + " .time");
    if (time_span.length == 0) {
      console.log("Can't find <span> element to receive time for resultid " + upd.getAttribute("resultid"));
    }
    time_span.html(upd.getAttribute("time"));
  }
}

// Process <has_new_schedule tbodyid="..."/> elements.
// newly_scheduled is a NodeList, not an array, with index identifying the
// current element.
function process_new_schedules(newly_scheduled, index, completed) {
  if (index < newly_scheduled.length) {
    var nsched = newly_scheduled.item(index);
	var tbodyid = nsched.getAttribute("tbodyid");
    console.log("Loading page section for tbody_" + tbodyid);

    // These assignments force curheat to get redone.
    g_update_status.current = {tbodyid: -1,
                               roundid: -1,
                               heat: -1};

	// The space in the load argument is significant!
	// The URL part presumably can't contain a space.
	$("#tbody_" + tbodyid)
        .load(location.href + " #tbody_" + tbodyid + " tr",
              /* data */'',
              function() {
                console.log("process_new_schedules: #tbody_" + tbodyid
                            + " now has " + $("#tbody_" + tbodyid).children().length
                            + " child(ren).");
                process_new_schedules(newly_scheduled, 1 + index, completed);
              });
  } else {
    completed();
  }
}

// Moves curgroup class and adds "Now Racing" text.
//
// tbodyid identifies the <tbody> element for the current round(s).
// round is the numeric round
// classname: optional name of current racing class
function notice_change_current_tbody(tbodyid, round, classname) {
  // g_update_status.current.tbodyid is initialized to -1, and query
  // poll.ondeck should keep reporting current.tbodyid=-1 before racing
  // starts.  While racing is actually underway, current.tbodyid should be > 0.
  // When concluded, current.tbodyid should report an empty string.
  if (tbodyid != g_update_status.current.tbodyid) {
	// Remove class and text from previous tbody, if any
	$(".curgroup").removeClass("curgroup");
	$(".pre_group_title").html("");

	// Mark the current racing tbody
	$("#tbody_" + tbodyid).addClass("curgroup");
    var curgroup = $(".curgroup .pre_group_title");
    if (curgroup.length > 0) {
	  curgroup.html("Now<br/>Racing");
    } else {
      console.log("Can't find title for new tbody " + tbodyid);
    }

	// Update text and target of "Now racing" link at top of page.
	if (tbodyid) {
	  $(".now_running").html("Now racing: <a href='#tbody_" + tbodyid + "'>"
							 + (classname ? classname + ", " : "")
							 + "Round " + round + "</a>");
	} else {
	  $(".now_running").html("Racing has concluded.");
	}
	g_update_status.current.heat = -2;
	g_update_status.current.tbodyid = tbodyid;
  }
}

// Move curheat css class if it's changed
//
// Looking for the #heat_{roundid}_{heat} row, if there is one.  (ondeck, but
// not racer-results.)  Even if we're using master scheduling, the row will be
// labeled with roundid, not tbodyid.
function notice_change_current_heat(roundid, heat) {
  // Scroll if necessary to see the heat AFTER current heat, but only if the
  // previously-current heat was visible.
  if (roundid != g_update_status.current.roundid ||
      heat != g_update_status.current.heat) {
	var vis = true;
	var curheat = $(".curheat");
	if (curheat.length > 0)
	  vis = is_visible(curheat[0]);
	curheat.removeClass("curheat");
	curheat = $("#heat_" + roundid + "_" + heat);
	curheat.addClass("curheat");
    $(".nextheat").removeClass("nextheat");
    var nextheat = $("#heat_" + roundid + "_" + (parseInt(heat) + 1));
    nextheat.addClass("nextheat");
	if (!nextheat[0]) {
	  nextheat = curheat;
	}
	if (vis && nextheat[0])
	  setTimeout(function() { scroll_to_current(nextheat[0]); }, 250);

	g_update_status.current.roundid = roundid;
	g_update_status.current.heat = heat;
  }
}

// Summary is the XML document element for the full <ondeck/> response.
function process_response_from_current(summary) {
  var current_xml = summary.getElementsByTagName("current-heat")[0];
  var current = {tbodyid: current_xml.getAttribute("tbodyid"),
                 roundid: current_xml.getAttribute("roundid"),
                 round: current_xml.getAttribute("round"),
                 heat: current_xml.getAttribute("heat"),
                 classname: current_xml.firstChild ? current_xml.firstChild.data : null};

  var nlanes = summary.getElementsByTagName("lanes")[0].getAttribute("n");

  var options = summary.getElementsByTagName("options")[0];
  var update_period = options.getAttribute("update-period");
  if (update_period != g_update_status.update_period) {
    g_update_status.update_period = update_period;
    clearInterval(g_update_status.interval_id);
    g_update_status.interval_id = start_polling();
  }

  if (typeof g_update_status.use_master_sched != 'undefined' && 
	  g_update_status.use_master_sched != options.getAttribute("use-master-sched")) {
	console.log("Reload triggered by: g_update_status.use_master_sched=" + g_update_status.use_master_sched +
				"; use-master-sched option = " + options.getAttribute("use-master-sched"));
	location.reload(true);
	return;
  }
  g_update_status.use_master_sched = options.getAttribute("use-master-sched");

  var high_water = summary.getElementsByTagName("high_water")[0];

  // Number of lanes may not have been known when the page was generated, but has since
  // been determined.  Update colspan attributes if that's the case.
  $(".wide").attr("colspan", nlanes);

  var new_high_water_tbodyid = high_water.getAttribute('tbodyid');
  if (new_high_water_tbodyid > g_update_status.high_water_tbodyid) {
    // If we have new rounds set up, just reload the whole page
    console.log("Reload triggered by g_update_status.high_water_tbodyid=" + g_update_status.high_water_tbodyid);
    location.reload(true);
    return;
  }

  process_new_schedules(summary.getElementsByTagName("has_new_schedule"),
                        0,
                        function () {
                          notice_change_current_tbody(current.tbodyid, current.round,
                                                      current.classname);
                          notice_change_current_heat(current.roundid, current.heat);

                          process_update_elements(summary.getElementsByTagName("update"));

                          g_update_status.last_update_time = high_water.getAttribute("completed");
                          g_update_status.high_water_resultid = high_water.getAttribute("resultid");
                        });
}

// True if any part of el is visible (vertically; doesn't check horizontally).
function is_visible(el) {
  var rect = el.getBoundingClientRect();
  return rect.bottom > 0 &&
         rect.top < $(window).height();
}


// If passed-in DOM element is below the viewport, then scrolls the viewport so
// the element is vertically centered on the display.
function scroll_to_current(el) {
  var rect = el.getBoundingClientRect();
  var w = $(window).height();

  if (rect.bottom > w) {  // Off-screen by being below.
    $(window).scrollTop($(window).scrollTop()
                        + (rect.top + rect.bottom)/2
                        - $(window).height()/2);
  }
}

// Returns interval id
function start_polling() {
    return setInterval(function() {
    console.log("Getting poll.ondeck since=" + g_update_status.last_update_time
                + " &high_water_resultid=" + g_update_status.high_water_resultid);
    $.ajax("action.php",
           {type: 'GET',
            data: {query: 'poll.ondeck',
                   since: g_update_status.last_update_time,
                   high_water_resultid: g_update_status.high_water_resultid,
                   merge_rounds: g_update_status.merge_rounds},
            success: function(data) {
              console.log(data);
              process_response_from_current(data);
            }
           });
  }, g_update_status.update_period);
}

$(document).ready(function() { g_update_status.interval_id = start_polling(); });
