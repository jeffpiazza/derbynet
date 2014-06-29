<script type="text/javascript">
// Mix-in script for managing the "<n> request(s) pending" box on a
// page that supports actions.
//
// NOTE: Doesn't handle reporting failed actions.
// NOTE: Only works with jquery ajax.
//
// TODO: Convert update.js to jquery ajax.
// TODO: Convert photo-thumbs page to new-style ajax.

var pending_ajax_requests = 0;

function ajax_add_request() {
  pending_ajax_requests += 1;
  $('#ajax_num_requests').html(pending_ajax_requests);
  $('#ajax_working').removeClass('hidden');
  console.log('ajax_add_request');
}

function ajax_remove_request() {
  pending_ajax_requests -= 1;
  $('#ajax_num_requests').html(pending_ajax_requests);
  if (pending_ajax_requests == 0) {
	$('#ajax_working').addClass('hidden');
  }
  console.log('ajax_remove_request');
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
