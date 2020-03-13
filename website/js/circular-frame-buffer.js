'use strict';

function CircularFrameBuffer(stream, no_seconds) {
  let debugging = false;

  let offscreen_video = document.createElement('video');
  offscreen_video.height = stream.getVideoTracks()[0].getSettings().height;
  offscreen_video.width = stream.getVideoTracks()[0].getSettings().width;

  let offscreen_canvas = document.createElement('canvas');
  offscreen_canvas.width = offscreen_video.width;
  offscreen_canvas.height = offscreen_video.height;
  let offscreen_context = offscreen_canvas.getContext('2d');

  $("#wh-stream").text(
    stream.getVideoTracks()[0].getSettings().width + "x" +
      stream.getVideoTracks()[0].getSettings().height);
  $("#wh-offscreen-video").text(
    offscreen_video.width + "x" +
      offscreen_video.height);
  $("#wh-window").text($(window).width() + "x" + $(window).height());
  $("#wh-offscreen-canvas").text(offscreen_canvas.width + "x" + offscreen_canvas.height);

  let frames;
  let frame_index = 0;
  let recording = false;

  offscreen_video.srcObject = stream;
  offscreen_video.play();

  function recording_callback(ts) {
    // ts is a double giving time in milliseconds
    //
    // offscreen_video.currentTime appears to advance continuously, and doesn't
    // provide information about video frame changes.

    offscreen_context.drawImage(offscreen_video, 0, 0);

    if (debugging) {
      offscreen_context.font = '20px serif';
      offscreen_context.fillStyle = 'red';
      let fr = '000' + frame_index;
      offscreen_context.fillText('Frame ' + fr.substr(fr.length - 3), 30, 40);
    }

    try {
      // frame_times[frame_index] = ts;
      frames[frame_index] =
        offscreen_context.getImageData(0, 0,
                                       offscreen_canvas.width,
                                       offscreen_canvas.height);
    } catch(error) {
      console.log("Caught error " + error.message);
    }

    frame_index = (frame_index + 1) % frames.length;

    if (recording) {
      requestAnimationFrame(recording_callback);
    }
  }

  this.start = function() {
    frames = Array(no_seconds * 60);
    // frame_times = Array(no_seconds * 60);
    frame_index = 0;
    recording = true;
    requestAnimationFrame(recording_callback);
  }

  this.stop = function() {
    recording = false;
  }

  this.width = function() { return offscreen_video.width; }
  this.height = function() { return offscreen_video.height; }

  // canvas -- a DOM <canvas> element
  // repeat -- number of times to play back the video
  // playback_rate -- multiplier for playback (0.5 = half speed slow-motion)
  // on_precanvas -- callback invoked on the offscreen <canvas> element rendering each frame
  // on_frame -- callback invoked on each frame, with findex argument
  // on_done -- callback to be invoked when playback completes.
  this.playback = function(canvas, repeat, playback_rate,
                           on_precanvas, on_frame, on_done) {
    if (!frames) {
      console.log("No frames!");
      return;
    }

    let context = canvas.getContext('2d');

    let pre_canvas = document.createElement('canvas');
    pre_canvas.width = offscreen_video.width;
    pre_canvas.height = offscreen_video.height;
    let pre_context = pre_canvas.getContext('2d');
    if (on_precanvas) {
      on_precanvas(pre_canvas);
    }

    let scale = Math.min(canvas.width / pre_canvas.width,
                         canvas.height / pre_canvas.height);
    let draw_width = pre_canvas.width * scale;
    let draw_height = pre_canvas.height * scale;
    let draw_x = (canvas.width - draw_width) / 2;
    let draw_y = (canvas.height - draw_height) / 2;

    // findex and last_frame_index are NOT mod frames.length
    let findex = frame_index + 1;
    let last_frame_index = frame_index + repeat * frames.length;

    // lastp and pindex ARE mod frames.length
    let lastp = frame_index;

    function playback_callback(ts) {
      try {
        let pindex = Math.round(findex) % frames.length;
        if (pindex != lastp) {
          pre_context.putImageData(frames[pindex], 0, 0);

          if (debugging) {
            pre_context.font = '20px serif';
            pre_context.fillStyle = 'yellow';
            let fr = '000' + ((Math.round(findex) - frame_index) % frames.length);
            pre_context.fillText('Index ' + fr.substr(fr.length - 3), 30, 80);
          }
          
          context.drawImage(pre_canvas, draw_x, draw_y, draw_width, draw_height);
          lastp = pindex;
        }
        if (on_frame) {
          on_frame((findex - frame_index) / frames.length);
        }
      } catch(e) {
        console.error(e);
      } finally {
        findex = findex + playback_rate;
        if (findex >= last_frame_index) {
          if (on_done) {
            on_done();
          }
          return;
        }
      }
      requestAnimationFrame(playback_callback);
    }

    requestAnimationFrame(playback_callback);
  }
}
