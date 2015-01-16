
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
                 console.log("Forcing a reload, because page (" + page
                             + ") != g_kiosk_page (" + g_kiosk_page + ")");
                 location.reload(true);
                 return;
               }
	           var reload = xmldoc.documentElement.getElementsByTagName("reload");
               if (reload && reload.length > 0) {
                 console.log("Forcing a reload because it was explicitly requested.");
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
