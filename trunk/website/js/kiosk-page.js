   // This script should get included by any page that's expected to operate under kiosk.php.
   // Note that $kiosk_page only gets set by kiosk.php, so g_kiosk_page is an empty string when used normally.
   var g_kiosk_page = "<?php echo isset($kiosk_page) ? $kiosk_page['page'] : ''; ?>";
   var g_kiosk_poll = Boolean(<?php echo isset($_GET['page']) ? 0 : 1; ?>);

// Fixed text from here on down
function kiosk_poll() {
  $.ajax('action.php',
         {type: 'GET',
           data: {query: 'kiosk-poll'},
           success: function(data) {
             setTimeout(kiosk_poll, 5000);
             $('#ajax_failure').addClass('hidden');
             $("#kiosk_name").text(data.documentElement.getAttribute("name"));
             if (g_kiosk_page != '') {
               var page = data.documentElement.getAttribute("page");
               if (page != g_kiosk_page) {
                 console.log("Forcing a reload, because page (" + page + ") != g_kiosk_page (" + g_kiosk_page + ")");
                 location.reload(true);
                 return;
               }
             }
           },
           error: function() {
             setTimeout(kiosk_poll, 5000);
             $('#ajax_status').html(this.status + " (" + 
                                    (this.status == 0 ? "likely timeout" : this.statusText)
                                    + ")");
             $('#ajax_failure').removeClass('hidden');
           }
         });
}


if (g_kiosk_poll) {
  $(document).ready(kiosk_poll);
} else {
  console.log("No g_kiosk_poll");
}
