'use strict';

// TODO stop() should perhaps return a Promise

// Starts capturing video from the stream argument, and returns a control
// object.  The returned object implements a stop(cb) method whose callback
// argument will be invoked with the recorded Blob.
function VideoCapture(stream) {
  let recorder = null;
  let recordedChunks = [];

  try {
    recorder = new MediaRecorder(stream, { /* mimeType : "video/webm" */ });
  } catch (e) {
    console.error('Exception while creating MediaRecorder: ' + e);
    return;
  }

  recorder.ondataavailable = (event) => {
    // event.data.type: video/x-matroska;codecs=avc1
    if (event.data.size > 0) {
      recordedChunks.push(event.data);
    }
  };

  recorder.start();

  this.stop = function(cb) {
    if (recorder.state == 'recording') {
      if (cb) {
        recorder.onstop = (event) => {
          if (recordedChunks.length > 0) {
            console.log("recorder.onstop with " + recordedChunks.length + " chunks");  // TODO
            let all_blobs = new Blob(recordedChunks, { mimeType: recordedChunks[0].type });
            recordedChunks = [];
            cb(all_blobs);
          }
        };
      }
      recorder.stop();
    } else {
      if (cb) {
        cb(null);
      }
    }
  };
}


// Keep a flight of n VideoCaptures, staggered at spacing_ms interval

function VideoCaptureFlight(stream, n, spacing_ms) {
  let captures = [];
  let interval = setInterval(function() {
    let new_capture;
    try {
        new_capture = new VideoCapture(stream);
    } catch (error) {
      console.log("Failed to create VideoCapture: " + error);
      clearInterval(interval);
    }
    if (captures.length >= n) {
      captures[0].stop();
      for (let i = 0; i < captures.length; ++i) {
        captures[i] = captures[i + 1];
      }
      captures[captures.length - 1] = new_capture;
    } else {
      captures.push(new_capture);
    }
  }, spacing_ms);

  this.stop = function(cb) {
    captures[0].stop(cb);
  };
}
