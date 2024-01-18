'use strict';


function PhotoCaptureModal() {
  // Default values for autocrop, depending on repo.  If user changes the setting,
  // that setting endures for the repo for as long as the page is loaded.
  var autocrop_head = true;
  var autocrop_car = false;
}

var g_photo_capture_modal = PhotoCaptureModal();
  



// TODO
// For the photo_modal dialog, this boolean controls whether the racer gets
// checked in as a side-effect of uploading a photo.
var g_check_in;

// TODO
// Default values for autocrop, depending on repo.  If user changes the setting,
// that setting endures for the repo for as long as the page is loaded.
var g_autocrop_head = true;
var g_autocrop_car = false;

var g_cameras = new Array();
var g_cameraIndex = 0;

function set_autocrop_state(repo) {
  $("#autocrop").prop('checked', repo == 'head' ? g_autocrop_head : g_autocrop_car);
  // TODO $("#autocrop").flipswitch("refresh");
}

function preserve_autocrop_state() {
  var repo = $("#photo_modal_repo").val();
  if (repo == 'head') {
    g_autocrop_head = $("#autocrop").prop('checked');
  } else {
    g_autocrop_car = $("#autocrop").prop('checked');
  }
}

// For #photo_drop form:
Dropzone.options.photoDrop = {
  paramName: 'photo',
  maxFiles: 1,
  maxFilesize: 8,
  url: 'action.php',
  acceptedFiles: 'image/*',
  // dropzone considers the upload successful as long as there was an HTTP response.  We need to look at the
  // message that came back and determine whether the file was actually accepted.
  sending: function(xhr, form_data) {
    preserve_autocrop_state();
  },
  success: function(file, data) {
    this.removeFile(file);

    if (data.hasOwnProperty('photo-url')) {
      var photo_url = data['photo-url'];
      $("#photo-" + $("#photo_modal_racerid").val()
        + " img[data-repo='" + $("#photo_modal_repo").val() + "']")
      .attr('src', photo_url);
    }

    close_modal("#photo_modal");
  },
};


window.addEventListener('orientationchange', function() {
});

function on_device_selection(selectq) {
  mobile_select_refresh(selectq);
  // If a stream is already open, stop it.
  var stream = document.getElementById("preview").srcObject;
  if (stream != null) {
    stream.getTracks().forEach(function(track) {
      track.stop();
    });
  }	

  let device_id = selectq.find(':selected').val();

  navigator.mediaDevices.getUserMedia(
    { video: {
      deviceId: device_id,
      // "ideal" 4K resolution is just an attempt to get the highest resolution
      // from the camera(s)
      width: { ideal: 3960 },
      // Including the height prompts Mac Chrome (at least) to offer 1080w x 1920h, which
      // isn't really desirable.
      // height: { ideal: 2160 }
    }
    })
    .then(function(stream) {
      // TODO
      for (var t = 0; t < stream.getVideoTracks().length; ++t) {
        console.log('  track', t, stream.getVideoTracks()[t].getSettings());
      }

      $("#preview")[0].srcObject = stream;
      try {
        console.log('Creating ImageCapture object');
        $("#preview").data('image-capture', new ImageCapture(stream.getVideoTracks()[0]));
      } catch(err) {
        console.error('new ImageCapture failed', err);
        $("#preview").data('image-capture', null);
      }
    });
}

$(function() {
  var previewj = $("#preview");
  var preview = previewj[0];
  preview.addEventListener('loadedmetadata',
                           (ev) => {
                             console.log('preview loadedmetadata', preview.videoWidth, preview.videoHeight);
                             if (preview.videoHeight == 0 || preview.videoWidth == 0) {
                               console.log('metadata says 0');
                               return;
                             }
                             // preview to be no bigger than 640w x 480h
                             var aspect = preview.videoWidth / preview.videoHeight;
                             if (aspect > 640 / 480) {
                               previewj.width(640);
                               previewj.height(640 / aspect);
                             } else {
                               previewj.height(480);
                               previewj.width(480 * aspect);
                             }
                             // Containing modal is 720px wide with 20px padding.
                             var margin = (720 - previewj.width()) / 2 - 20;
                             previewj.css({'margin-left': margin,
                                                'margin-right': margin});
                           });
});

function show_photo_modal(racerid, repo) {
  var firstname = $('#firstname-' + racerid).text();
  var lastname = $('#lastname-' + racerid).text();
  $("#racer_photo_name").text(firstname + ' ' + lastname);
  $("#racer_photo_repo").text(repo);
  $("#photo_modal_repo").val(repo);
  $("#photo_modal_racerid").val(racerid);

  set_autocrop_state(repo);

  $("#thumb-link").attr('href',
                        'photo-thumbs.php?repo=' + repo +
                        '&racerid=' + racerid +
                        '&back=checkin.php');

  // If the racer's already been checked in, don't offer "Capture & Check In" button
  $("#capture_and_check_in").toggleClass('hidden', $("#passed-" + racerid).prop('checked'));

  // TODO Two different submit buttons that set a global, g_check_in.  Eww.
  show_modal("#photo_modal", function() {
    preserve_autocrop_state();
    take_snapshot(racerid, repo, lastname + '-' + firstname);
    return false;
  });

  if (typeof(navigator.mediaDevices) == 'undefined') {
    $("#no-camera-warning").removeClass('hidden');
    if (window.location.protocol == 'http:') {
      var https_url = "https://" + window.location.hostname + window.location.pathname;
      $("#no-camera-warning").append("<p>You may need to switch to <a href='" +  https_url + "'>" + https_url + "</a></p>");
    }
  } else {
    navigator.mediaDevices.ondevicechange = function(event) {
      build_device_picker($("#device-picker"), /*include_remote*/false, on_device_selection);
    };
  }

  build_device_picker($("#device-picker"), /*include_remote*/false, on_device_selection);
}

function show_racer_photo_modal(racerid) {
  show_photo_modal(racerid, 'head');
}
function show_car_photo_modal(racerid) {
  console.log('show_car_photo_modal', racerid);
  show_photo_modal(racerid, 'car');
}


// =================

function onTakePhotoButtonClick() {
  imageCapture.takePhoto()
  .then((blob) => createImageBitmap(blob))
  .then((imageBitmap) => {
    const canvas = document.querySelector('#takePhotoCanvas');
    drawCanvas(canvas, imageBitmap);
  })
  .catch((error) => console.error(error));
}

/* Utils */

function drawCanvas(canvas, img) {
  canvas.width = getComputedStyle(canvas).width.split('px')[0];
  canvas.height = getComputedStyle(canvas).height.split('px')[0];
  let ratio  = Math.min(canvas.width / img.width, canvas.height / img.height);
  let x = (canvas.width - img.width * ratio) / 2;
  let y = (canvas.height - img.height * ratio) / 2;
  canvas.getContext('2d').clearRect(0, 0, canvas.width, canvas.height);
  canvas.getContext('2d').drawImage(img, 0, 0, img.width, img.height,
      x, y, img.width * ratio, img.height * ratio);
}


function onGrabFrameButtonClick() {
  imageCapture.grabFrame()
  .then(imageBitmap => {
    const canvas = document.querySelector('#grabFrameCanvas');
    drawCanvas(canvas, imageBitmap);
  })
  .catch(error => ChromeSamples.log(error));
}

function onTakePhotoButtonClick() {
  imageCapture.takePhoto()
  .then(blob => createImageBitmap(blob))
  .then(imageBitmap => {
    const canvas = document.querySelector('#takePhotoCanvas');
    drawCanvas(canvas, imageBitmap);
  })
  .catch(error => ChromeSamples.log(error));
}
// =================


function take_snapshot(racerid, repo, photo_base_name) {
  if (photo_base_name.length <= 1) {
    photo_base_name = 'photo';
  }

  // g_check_in set by onclick method in submit buttons
  if (g_check_in) {
    $("#passed-" + racerid).prop('checked', true);
    $("#passed-" + racerid).trigger("change", true);

    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'racer.pass',
                   racer: racerid,
                   value: 1},
           });
  }

  var canvas = document.createElement('canvas');
  if ($("#preview").data('image-capture')) {
    console.log('image-capture exists!');
    $("#preview").data('image-capture').grabFrame()
      .then((image_bitmap) => {
        console.info('image_bitmap is ', image_bitmap.width, image_bitmap.height);
        console.info(image_bitmap);
        canvas.width = image_bitmap.width;
        canvas.height = image_bitmap.height;
        canvas.getContext('2d').drawImage($("#preview")[0], 0, 0,
                                          canvas.width, canvas.height);
        send_canvas_photo(canvas, racerid, repo, photo_base_name);
      })
      .catch((error) => {
        console.error('grabFrame error', error);
        // If we didn't get an image bitmap (or had some other problem), then fall
        // back to just capturing the preview window content.
        send_photo_from_preview(canvas, racerid, repo, photo_base_name);
      });
  } else {
    send_photo_from_preview(canvas, racerid, repo, photo_base_name);
  }
}

function send_photo_from_preview(canvas, racerid, repo, photo_base_name) {
  console.error('Falling back to preview capture', $("#preview").width(), $("#preview").height());
  canvas.width = $("#preview").width();
  canvas.height = $("#preview").height();
  canvas.getContext('2d').drawImage($("#preview")[0], 0, 0, canvas.width, canvas.height);
  send_canvas_photo(canvas, racerid, repo, photo_base_name);
}

function send_canvas_photo(canvas, racerid, repo, photo_base_name) {
  var image_data_url = canvas.toDataURL('image/jpeg');
  canvas.remove();

  // https://stackoverflow.com/questions/6850276
  
  // convert base64 to raw binary data held in a string
  // doesn't handle URLEncoded DataURIs - see SO answer #6850276 for code that does this
  var byteString = atob(image_data_url.split(',')[1]);

  // write the bytes of the string to an ArrayBuffer
  var ab = new ArrayBuffer(byteString.length);
  var ia = new Uint8Array(ab);
  for (var i = 0; i < byteString.length; i++) {
    ia[i] = byteString.charCodeAt(i);
  }
  var blob = new Blob([ab], {type: 'image/jpeg'});

  if (blob.size == 0) {
    $("#ajax-failure").text("Photo capture failure").removeClass('hidden');
    setTimeout(function() { $("#ajax-failure").addClass('hidden'); }, 10000);
    return;
  }

  // stuff into a form, so servers can easily receive it as a standard file upload
  var form_data = new FormData();
  form_data.append('action', 'photo.upload');
  form_data.append('racerid', racerid);
  form_data.append('repo', repo);
  form_data.append('photo', blob, photo_base_name + ".jpg");
  if ($("#autocrop").prop('checked')) {
    form_data.append('autocrop', '1');
  }

  // Testing for <failure> elements occurs in dashboard-ajax.js
  $.ajax(g_action_url,
         {type: 'POST',
          data: form_data,
          contentType: false,
          processData: false,
          success: function(data) {
            if (data.hasOwnProperty('photo-url')) {
              var photo_url = data['photo-url'];
              $("#photo-" + racerid + " img[data-repo='" + repo + "']").attr('src', photo_url);
            }
          }
         });

  // TODO-Webcam.reset();
  close_modal("#photo_modal");
  }
  
function close_photo_modal() {
  // TODO-Webcam.reset();
  close_modal("#photo_modal");
}
