// var g_kiosk_page 

function kiosk_poll_ready() {
  if (this.readyState == this.DONE) {
	if (this.status == 200) {
	  $('#ajax_failure').addClass('hidden');
	  var poll = this.responseXML.documentElement;
	  var page = poll.getAttribute("page");
	  if (page != g_kiosk_page) {
		location.reload(true);
		return;
	  }
    } else {
	  $('#ajax_status').html(this.status + " (" + 
							 (this.status == 0 ? "likely timeout" : this.statusText)
							 + ")");
	  $('#ajax_failure').removeClass('hidden');
	}
    setTimeout(kiosk_poll_fire, 5000);
  }
}

function kiosk_poll_fire() {
  var xmlhttp = new XMLHttpRequest();
  xmlhttp.open("GET", "action.php?query=kiosk-poll", /*async*/true);
  xmlhttp.onreadystatechange = kiosk_poll_ready;
  xmlhttp.send("");
}

if (g_kiosk_poll) {
  $(document).ready(kiosk_poll_fire);
} else {
  console.log("No g_kiosk_poll");
}

