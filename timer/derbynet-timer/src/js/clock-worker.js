'use strict';

// setTimeout (and setInterval) in the main thread may allow longer-than-desired
// sleep if the timer window loses focus.  Moving setTimeout calls to a web worker
// is more reliable.

// From the main thread, g_clock_worker.postMessage([ key, delay, values... ]).
//  key  is a string to distinguish this timeout from others.  Only one timeout can
//       be pending for a given key.  If key is null, the timeout is anonymous and
//       will not be cancelable.
//  delay in ms
//
//  After the timeout expires, the values get passed back to the main thread.
//
// g_clock_worker.postMessage([ key ]) cancels a previously-scheduled timeout.

var g_timers = {};

onmessage = function(e) {
  var key = e.data.shift();

  if (key === null) {
    // An keyless message just gets posted as-is, without replacing any other timeouts
    var delay = e.data.shift();
    var msg = e.data;
    setTimeout(function() { postMessage(msg); }, delay);
  } else {
    if (g_timers.hasOwnProperty(key)) {
      clearTimeout(g_timers[key]);
      delete g_timers[key];
    }
    if (e.data.length >= 2) {
      var started = Date.now();
      var delay = e.data.shift();
      var msg = e.data;
      g_timers[key] = setTimeout(function() {
        if (false) {
          console.log('clock-worker: asked-for delay', delay,
                      'actual delay: ', Date.now() - started,
                      'delta: ', Date.now() - started - delay);
        }
        delete g_timers[key];
        postMessage(msg);
      }, delay);
    }
  }
}


setInterval(function () { postMessage(['LOGGER']); }, 250);
