// Assumes ajax-failure.inc has already established a global ajax error handler

var g_kiosk_poll_interval = null;

function kiosk_poll() {
  $.ajax('action.php',
         {type: 'GET',
           data: {query: 'kiosk-poll'},
           success: function(data) {
             cancel_ajax_failure();
             $("#kiosk_name").text(data.documentElement.getAttribute("name"));
             if (g_kiosk_page != '') {
               var page = data.documentElement.getAttribute("page");
               if (page != g_kiosk_page) {
                 console.log("Forcing a reload, because page (" + page
                             + ") != g_kiosk_page (" + g_kiosk_page + ")");
                 clearInterval(g_kiosk_poll_interval);
                 location.reload(true);
                 return;
               }
	           var reload = data.documentElement.getElementsByTagName("reload");
               if (reload && reload.length > 0) {
                 console.log("Forcing a reload because it was explicitly requested.");
                 clearInterval(g_kiosk_poll_interval);
                 location.reload(true);
                 return;
               }
             }
           }
         });
}


if (g_kiosk_poll) {
    $(document).ready(function() {
        g_kiosk_poll_interval = setInterval(kiosk_poll, 5000);
    });
} else {
  console.log("No g_kiosk_poll, so not polling for new pages");
}
