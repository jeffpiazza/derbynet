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
  if (tbodyid != $("tbody.curgroup").attr('id')) {
	// Remove class and text from previous tbody, if any
	$(".curgroup").not("#overflow").removeClass("curgroup");
	$(".pre_group_title").html("");

	// Mark the current racing tbody
	$("#tbody_" + tbodyid).addClass("curgroup");
    var curgroup = $(".curgroup .pre_group_title");
    if (curgroup.length > 0) {
	  curgroup.html("Now<br/>Racing");
    } else {
      // TODO console.log("Can't find title for new tbody " + tbodyid);
    }

	// Update text and target of "Now racing" link at top of page.
	if (tbodyid) {
	  $(".now_running").html("Now racing: <a href='#tbody_" + tbodyid + "'>"
							 + (classname ? classname + ", " : "")
							 + "Round " + round + "</a>");
	} else {
	  $(".now_running").html("Racing has concluded.");
	}
  }
}
