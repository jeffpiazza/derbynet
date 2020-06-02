'use strict';

var g_offscreen_video;
var g_offscreen_canvas;

function CircularFrameBuffer(stream, no_seconds) {
  // We expect a video refresh rate of 60 frames per second
  const k_refresh_fps = 60;
  let debugging = false;

  let resizing_callback = false;
  this.on_resize = function(cb) { resizing_callback = cb; }
  
  let offscreen_video = document.createElement('video');
  // For a remote stream, the width and height may initially not be set, and
  // will require fixing up in the catch clause in recording_playback.
  let stream_settings = stream.getVideoTracks()[0].getSettings();
  offscreen_video.width = stream_settings.width;
  offscreen_video.height = stream_settings.height;

  let offscreen_canvas = document.createElement('canvas');
  offscreen_canvas.width = offscreen_video.width;
  offscreen_canvas.height = offscreen_video.height;
  let offscreen_context = offscreen_canvas.getContext('2d');

  let frames;
  let frame_times;
  // frame_index is the index into frames[] and frame_times[] for the NEXT frame
  // to capture.
  let frame_index = 0;
  // last_recorded_frame_index, if >= 0, is the index of the most recently captured
  // frame.
  let last_recorded_frame_index = -1;
  let recording = false;

  offscreen_video.srcObject = stream;
  offscreen_video.play();

  let fps_div = $("#fps");
  let last_ts = 0;
  function report_fps(ts, play_or_rec) {
    let delta = ts - last_ts;
    last_ts = ts;
    if (delta != 0) {
      let fps = Math.round(1000 / delta);
      fps_div.css("background-color", fps < 30 ? "red" : (fps < k_refresh_fps ? "yellow" : "white"))
        .toggleClass('hidden', fps >= k_refresh_fps)
        .text(play_or_rec + fps + " fps");
    }
  }

  function recording_callback(ts) {
    // ts is a double giving time in milliseconds
    //
    // offscreen_video.currentTime appears to advance continuously, and doesn't
    // provide information about video frame changes.
    report_fps(ts, "rec ");

    // Only capture at 30fps
    if (last_recorded_frame_index < 0 || ts - frame_times[last_recorded_frame_index] >= 33) {
      offscreen_context.drawImage(offscreen_video, 0, 0);

      if (debugging) {
        offscreen_context.font = '20px serif';
        offscreen_context.fillStyle = 'red';
        let fr = '000' + frame_index;
        offscreen_context.fillText('Frame ' + fr.substr(fr.length - 3), 30, 40);
      }

      try {
        let tr = stream.getVideoTracks()[0];
        if (tr) {
          let settings = tr.getSettings();
          let did_resize = false;
          if (settings.width && offscreen_canvas.width != settings.width) {
            console.log("Adjusting width to " + settings.width);
            offscreen_video.width = offscreen_canvas.width = settings.width;
            did_resize = true;
          }

          if (settings.height && offscreen_canvas.height != settings.height) {
            console.log("Adjusting height to " + settings.height);
            offscreen_video.height = offscreen_canvas.height = settings.height;
            did_resize = true;
          }
          if (did_resize && resizing_callback) {
            resizing_callback(offscreen_video.width, offscreen_video.height);
          }
        }

        frames[frame_index] =
          offscreen_context.getImageData(0, 0,
                                         offscreen_canvas.width,
                                         offscreen_canvas.height);
        frame_times[frame_index] = ts;
        last_recorded_frame_index = frame_index;
      } catch(error) {
        console.log("Caught error " + error.message);
        // For a remote stream, the width and height may not have been known
        // initially, and require fixing up here.
      }

      frame_index = (frame_index + 1) % frames.length;
    }

    if (recording) {
      requestAnimationFrame(recording_callback);
    }
  }

  this.start = function() {
    frames = Array(no_seconds * k_refresh_fps);
    frame_times = Array(no_seconds * 60);
    frame_index = 0;
    last_recorded_frame_index = -1;
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
  // on_playback_finished -- callback invoked when a playback finishes (may be called repeat times)
  // on_done -- callback to be invoked when playback completes.
  this.playback = function(canvas, repeat, playback_rate,
                           on_precanvas, on_playback_finished, on_done) {
    if (!frames) {
      console.log("No frames for playback!");
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

    if (last_recorded_frame_index < 0) {
      console.log("No captured frames!");
      return;
    }

    // frame_times[last_recorded_frame_index] is the time of the last captured frame
    // frame_times[last_recorded_frame_index] - no_seconds * 1000 is the time of the first frame for playback
    let start_goal = frame_times[last_recorded_frame_index] - no_seconds * 1000;
    console.log("Playback: repeat=" + repeat + ", playback_rate=" + playback_rate);
    console.log("last_recorded_frame_index = " + last_recorded_frame_index);
    console.log("Last frame time = " + frame_times[last_recorded_frame_index]);
    console.log("Start goal = " + start_goal);
    
    let start_index = -1;
    for (let step = 0; step < frames.length; ++step) {
      let pindex = (last_recorded_frame_index + frames.length - step) % frames.length;
      if (frame_times[pindex] < start_goal) {
        break;
      }
      start_index = pindex;
    }

    console.log("start_index = " + start_index);

    // During playback, pindex runs from start_index to last_recorded_frame_index, circularly,
    // inclusive, but may skip steps.

    let pindex;
    let last_shown_frame;
    let playback_start_time;

    let rpt = 0;

    function start_playback() {
      pindex = start_index;
      last_shown_frame = -1;
      playback_start_time = performance.now();
      requestAnimationFrame(playback_callback);
    }

    function playback_callback(ts) {
      report_fps(ts, "play ");

      let goal_frame_time = start_goal + (ts - playback_start_time) * playback_rate;
      let found = false;
      for (let tries = 0; tries < frames.length; ++tries) {
        if (frame_times[pindex] >= goal_frame_time) {
          found = true;
          break;
        }
        if (pindex == last_recorded_frame_index) {
          break;
        }
        pindex = (pindex + 1) % frames.length;
      }
      if (found) {
        if (pindex == last_shown_frame) {
          //console.log("Skipping currently-displayed frame pindex = " + pindex +
          //            " at time " + frame_times[pindex] +
          //            " for goal " + goal_frame_time);
        } else {
          //console.log("Found pindex = " + pindex + " at time " + frame_times[pindex] +
          //            " for goal " + goal_frame_time);
          try {
            pre_context.putImageData(frames[pindex], 0, 0);
            context.drawImage(pre_canvas, draw_x, draw_y, draw_width, draw_height);
          } catch(e) {
            console.error(e);
          }

          last_shown_frame = pindex;
        }

        requestAnimationFrame(playback_callback);
      } else {
        console.log("Playback done (once)!");

        if (on_playback_finished) {
          try {
            on_playback_finished();
          } catch(e) {
            console.error(e);
          }
        }

        ++rpt;
        if (rpt < repeat) {
          start_playback();
        } else {
          if (on_done) {
            on_done();
          }
        }
      }
    }

    start_playback();
  }
}
