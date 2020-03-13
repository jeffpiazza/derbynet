'use strict';

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
            let all_blobs = new Blob(recordedChunks, { mimeType: recordedChunks[0].type });
            recordedChunks = [];
            cb(all_blobs);
          } else {
            cb(null);
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
