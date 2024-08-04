'use strict';

// This file defines a Worker thread that is initialized in main.js by:
//    g_clock_worker = new Worker('js/timer/clock-worker.js');
//    g_clock_worker.onmessage = ... // function that will receive an event posted FROM clock worker
//
// [https://developer.mozilla.org/en-US/docs/Web/API/Web_Workers_API/Using_web_workers:]
// Data is sent between workers and the main thread via a system of messages —
// both sides send their messages using the postMessage() method, and respond to
// messages via the onmessage event handler (the message is contained within the
// message event's data attribute). The data is copied rather than shared.
//////////////////////////////////////////////////////////////////////////////////
//
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

// Calls to g_cloud_worker.postMessage will copy the message to the worker
// thread and invoke this function:
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
        // This will transfer msg to the main thread and invoke
        // g_clock_worker.onmessage there.
        postMessage(msg);
      }, delay);
    }
  }
}


setInterval(function () { postMessage(['LOGGER']); }, 250);
