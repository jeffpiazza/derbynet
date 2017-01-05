/**
 * Wait until the test condition is true or a timeout occurs. Useful for waiting
 * on a server response or for a ui change (fadeIn, etc.) to occur.
 *
 * @param testFx javascript callback function that evaluates to a boolean
 * @param onReady: a callback function, what to do when testFx condition is fulfilled.
 * @param timeOutMillis the max amount of time to wait. If not specified, 3 sec is used.
 */

"use strict";
exports.for = function(testFx, onReady, timeOutMillis) {
    var maxtimeOutMillis = timeOutMillis ? timeOutMillis : 3000, //< Default Max Timout is 3s
        start = new Date().getTime(),
        condition = false,
        interval = setInterval(function() {
            if ( (new Date().getTime() - start < maxtimeOutMillis) && !condition ) {
                // If not time-out yet and condition not yet fulfilled
                condition = (typeof(testFx) === "string" ? eval(testFx) : testFx()); //< defensive code
            } else {
                if(!condition) {
                    // If condition still not fulfilled (timeout but condition is 'false')
                    console.log("'wait.for()' timeout");
                    phantom.exit(1);
                } else {
                    // Condition fulfilled (timeout and/or condition is 'true')
                  onReady(); //< Do what it's supposed to do once the condition is fulfilled
                  clearInterval(interval); //< Stop this interval
                }
            }
        }, 250); //< repeat check every 250ms
};

