// Assumes ajax-failure.inc has already established a global ajax error handler

// KioskPoller.start(address, kiosk_page) starts the polling loop; see inc/kiosk-poller.inc
//
// On each cycle, KioskPoller.param_callback gets invoked with the current param
// string; parameter-aware kiosk pages should assign KioskPoller.param_callback
// to track parameter changes.

var KioskPoller = (function(KioskPoller) {

  KioskPoller.param_callback = function(parameters) {
    // console.log("Params: " + param_string);
  };

  KioskPoller.start = function(address, kiosk_page) {
    var interval = setInterval(function() {
      $.ajax('action.php',
             {type: 'GET',
              data: {query: 'poll.kiosk',
                     address: address},
              success: function(data) {
                if (data["cease"]) {
                  clearInterval(interval);
                  window.location.href = '../index.php';
                  return;
                }
                var setting = data['kiosk-setting'];
                cancel_ajax_failure();
                $("#kiosk_name").text(setting.name);
                var page = setting.page;
                if (page != kiosk_page) {
                  console.log("Forcing a reload, because page (" + page
                              + ") != current kiosk_page (" + kiosk_page + ")");
                  location.reload(true);
                  return;
                }
	            if (setting.reload) {
                  console.log("Forcing a reload because it was explicitly requested.");
                  location.reload(true);
                  return;
                }
                var params_string = '{}';
                if (setting.params.length !== 0) {
                  console.log("setting.params=" + setting.params)
                  params_string = setting.params;
                }
                KioskPoller.param_callback(JSON.parse(params_string));
              }
             });
    }, 5000);
  }

  return KioskPoller;
}(KioskPoller || {}));

