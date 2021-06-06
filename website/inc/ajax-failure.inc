<script type="text/javascript">
// Ajax failure handling, via a (provided) #ajax-failure div, suitable for
// unattended kiosks.
//
// Include by having a &lt;? php require('inc/ajax-failure.inc'); ?&gt;
// line in the BODY of the page.

// This variable gets set to true when leaving a page; it should prevent errors from
// showing for any in-flight ajax calls.
var g_unloading = false;

$(window).bind("beforeunload", function() { g_unloading = true; });


function ajax_failure(status, statusText, errorThrown) {
  $('#ajax_status').html(status + " ("
                         + (errorThrown != '' ? errorThrown + ' / ' : '')
                         + (status == 0 ? "likely timeout" : statusText)
                         + ")");
  $('#ajax-failure').removeClass('hidden');
}

function cancel_ajax_failure() {
  $('#ajax-failure').addClass('hidden');
}

$(document).ajaxError(function(event, jqxhr, ajaxSettings, thrownError) {
  console.log('ajaxError fired!');
  console.log(event);
  console.log(jqxhr);
  if (!g_unloading) {
    ajax_failure(jqxhr.status, jqxhr.statusText, thrownError);
    $('#ajax-failure').removeClass('hidden');
    setTimeout(function() { $('#ajax-failure').addClass('hidden'); }, 10000);
  }
});
</script>

<div id="ajax-failure" class="hidden">
  Ajax request failed with <span id="ajax_status">0</span>.
</div>
