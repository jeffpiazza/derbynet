<script type="text/javascript">
 $(document).ajaxError(function(event, jqxhr, ajaxSettings, thrownError) {
     console.log('ajaxError fired!');
    $('#ajax_status').html(jqxhr.status + " (" + thrownError + " / " +
        (jqxhr.status == 0 ? "likely timeout" : jqxhr.statusText)
        + ")");
    $('#ajax_failure').removeClass('hidden');
    setTimeout(function() { $('#ajax_failure').addClass('hidden'); }, 10000);
});
</script>

<div id="ajax_failure" class="hidden">
  Ajax request failed with <span id="ajax_status">0</span>.
</div>
