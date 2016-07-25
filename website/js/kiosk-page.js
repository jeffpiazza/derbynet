// Assumes ajax-failure.inc has already established a global ajax error handler

function kiosk_poll(address, kiosk_page) {
  setInterval(function() {
    console.log("Kiosk-poll " + address + " " + kiosk_page);
    $.ajax('action.php',
           {type: 'GET',
            data: {query: 'kiosk.poll',
                   address: address},
            success: function(data) {
              cancel_ajax_failure();
              $("#kiosk_name").text(data.documentElement.getAttribute("name"));
              var page = data.documentElement.getAttribute("page");
              if (page != kiosk_page) {
                console.log("Forcing a reload, because page (" + page
                            + ") != current kiosk_page (" + kiosk_page + ")");
                location.reload(true);
                return;
              }
	          var reload = data.documentElement.getElementsByTagName("reload");
              if (reload && reload.length > 0) {
                console.log("Forcing a reload because it was explicitly requested.");
                location.reload(true);
                return;
              }
            }
           });
  }, 5000);
}
