<script type="text/javascript">
// Mix-in script for managing the "((n)) request(s) pending" box on a
// page that supports actions.
//
// NOTE: Doesn't handle reporting failed actions.
// NOTE: Only works with jquery ajax.
//
// TODO: Convert update.js to jquery ajax.
// TODO: Convert photo-thumbs page to new-style ajax.

var pending_ajax_requests = 0;

function ajax_add_request() {
  var first = (pending_ajax_requests == 0);
  pending_ajax_requests += 1;
  $('#ajax_num_requests').html(pending_ajax_requests);
  if (first) {
    // To avoid the #ajax_working bug flickering on and off for ordinary
    // requests that don't hang, delay exposing for a half-second.
    setTimeout(function() {
      if (pending_ajax_requests > 0) {
        $('#ajax_working').removeClass('hidden');
      }
    }, 500);
  } else {
    // Just to be sure
    $('#ajax_working').removeClass('hidden');
  }
}

function ajax_remove_request() {
  pending_ajax_requests -= 1;
  $('#ajax_num_requests').html(pending_ajax_requests);
  if (pending_ajax_requests == 0) {
	$('#ajax_working').addClass('hidden');
  }
}

$(document).ajaxSend(function(event, xhr, options) {
    ajax_add_request();
});

$(document).ajaxComplete(function(event, xhr, options) {
    ajax_remove_request();
});

</script>

<div id="ajax_working" class="hidden">
  <span id="ajax_num_requests">0</span> request(s) pending.
</div>
