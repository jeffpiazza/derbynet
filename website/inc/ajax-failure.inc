<script type="text/javascript">
 // Ajax failure handling, via a (provided) #ajax-failure div, suitable for
 // unattended kiosks.
 //
 // Include by having a &lt;? php require('inc/ajax-failure.inc'); ?&gt;
 // line in the BODY of the page.

 $(document).ajaxError(function(event, jqxhr, ajaxSettings, thrownError) {
     console.log('ajaxError fired!');
    $('#ajax_status').html(jqxhr.status + " (" + thrownError + " / " +
        (jqxhr.status == 0 ? "likely timeout" : jqxhr.statusText)
        + ")");
    $('#ajax-failure').removeClass('hidden');
    setTimeout(function() { $('#ajax-failure').addClass('hidden'); }, 10000);
});

function ajax_failure(status, statusText) {
  $('#ajax_status').html(status + " ("
                         + (status == 0 ? "likely timeout" : statusText)
                         + ")");
  $('#ajax-failure').removeClass('hidden');
}

function cancel_ajax_failure() {
  $('#ajax-failure').addClass('hidden');
}
</script>

<div id="ajax-failure" class="hidden">
  Ajax request failed with <span id="ajax_status">0</span>.
</div>
