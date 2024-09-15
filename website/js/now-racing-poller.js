
var Poller = {
  // Timestamp when the last polling request was sent; used by watchdog to
  // detect a failure to queue a new request.
  time_of_last_request: 0,

  // Records the identifier of the timeout that will send the next polling
  // request, or 0 if no request is queued.
  id_of_timeout: 0,

  // If we're being shown within a replay iframe, suspend polling while a replay
  // is showing and we're not visible; resume when we have the display again.
  // We keep setting timeouts (which makes the watchdog happy), but when
  // suspended, poll_for_update just queues another request rather than asking
  // the server for anything.
  suspended: false,

  // Set if the server has asked us to cease polling
  ceased: false,

  // This gets overridden in the actual now-racing kiosks
  build_request: function(roundid, heat) {
    return {query: 'poll',
            values: '',
            roundid: roundid,
            heat: heat};
  },
  
  // Queues the next poll request when processing of the current request has
  // completed, including animations, if any.  Because animations are handled
  // asynchronously, with completion callbacks, we can't just start the next
  // request when process_now_racing_element returns.
  queue_next_request: function(roundid, heat) {
    if (this.id_of_timeout != 0) {
      console.log("Trying to queue a polling request when there's already one pending; ignored.");
    } else {
      Poller.id_of_timeout = setTimeout(function() {
        Poller.id_of_timeout = 0;
        Poller.poll_for_update(roundid, heat);
      }, 500);  // 0.5 sec
    }
  },

  poll_for_update: function(roundid, heat) {
    if (typeof(simulated_poll_for_update) == "function") {
      simulated_poll_for_update();
    } else if (this.suspended) {
      Poller.queue_next_request(roundid, heat);
    } else if (this.ceased) {
    } else {
      this.time_of_last_request = Date.now();
      $.ajax('action.php',
             {type: 'GET',
              data: Poller.build_request(roundid, heat),
              success: function(data) {
                if (data["cease"]) {
                  Poller.ceased = true;
                  window.location.href = '../index.php';
                  return;
                }
                process_polling_result(data);
              },
              error: function() {
                Poller.queue_next_request(roundid, heat);
              }
             }
            );
    }
  },

  // Correct operation depends on queuing a new request when we're done
  // processing the last one, including any animations (could take a few
  // seconds).  As a safeguard against a bug that perhaps prevents queuing the
  // next request, the watchdog periodically tests whether one seems overdue,
  // and may queue a new request if so.
  watchdog: function() {
    if (this.id_of_timeout != 0 && this.time_of_last_request + 15000 < Date.now()) {
      console.error("Watchdog notices no requests lately, and none queued!");
      this.queue_next_request();
    }
  }
};
