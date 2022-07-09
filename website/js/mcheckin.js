

$(function() {
  setTimeout(function() { on_recognized_barcode({text: 'PWDid003'}); }, 2000);

  $("#controls-inner").css({'margin-left': ($(window).width() - $("#controls-inner").width()) / 2});

  console.log('Preview bottom', ($(window).height() - 110),
              'expected height', $(window).height() - 170);
  console.log('Jquery window', $(window).width(), $(window).height());
  console.log('window.inner', window.innerWidth, window.innerHeight);
});

var codeReader = new ZXingBrowser.BrowserMultiFormatOneDReader();

// Last scanned barcode value
var g_barcode;

async function on_device_selection(selectq) {
  // If a stream is already open, stop it.
  stream = document.getElementById("preview").srcObject;
  if (stream != null) {
    stream.getTracks().forEach(function(track) {
      track.stop();
    });
  }	

  let device_id = selectq.find(':selected').val();
  let video_constraint = {deviceId: device_id,
                          // width: $("#preview").width(),
                          // height: $("#preview").height()
                         };

  navigator.mediaDevices.getUserMedia({video: video_constraint})
    .then(function(stream) {
      var tracks = stream.getTracks();
      if (tracks.length < 1) {
        return;
      }
      var settings = tracks[0].getSettings();

      // Max height available
      var max_height = $(window).height() - $("#banner").height() - /* slide-up height */110;

      console.log('Settings', settings);
      console.log('Available space (w,h)', $(window).width(), max_height);
      console.log('Height required at max width', $(window).width() / settings.aspectRatio);

      $("#preview").css({'min-height': '',
                         'min-width': '',
                         'left': '',
                         'height': ''});
      if (max_height >= $(window).width() / settings.aspectRatio) {
        // Display at max width in order not to be too tall
        $("#preview").css({'min-height': 0, 'min-width': '100%', 'left': 0});
      } else {
        // Restrict to max height
        var width = max_height * settings.aspectRatio;
        $("#preview").css({'min-height': max_height,
                           'height': max_height,
                           'min-width': width + 'px',
                           'left': ($(window).width() - width) / 2,
                          });
      }

      codeReader.decodeFromStream(stream, 'preview',
                                  function(result, error, control) {
                                    if (result) {
                                      on_recognized_barcode(result);
                                    } else if (error && error.getKind() != "NotFoundException") {
                                      console.log('error', error.getKind(), error);
                                    }
                                  });
    });
}

function on_recognized_barcode(result) {
  if (result.text == g_barcode) {
    return;
  }

  g_barcode = result.text;
  $("#slide-up").css('height', '0px');
  $("#flash-overlay").css({display: 'block'}).fadeOut();
  $("#beep")[0].play();

  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'racer.list',
                 barcode: result.text,
                 render: '150x150',  // Thumbnail size
                },
          success: function(json) {
            // firstname, lastname, carnumber, carname, note_from, headshot, car_photo,
            // classid, class, rankid, rank, passed, seq
            // TOOD partitionid
            console.log(json);
            var racer = json['racers'][0];
            $("#racer-div").text(racer.firstname + " " + racer.lastname +
                                 "  " + racer.carnumber + " " + racer.carname);
            update_thumbnail('head', racer.headshot);
            update_checked_in(racer.passed);
            update_thumbnail('car',  racer.carphoto);
            $("#slide-up").animate({height: '110px'});
          }
         });
}


$(function() {
  if (typeof(navigator.mediaDevices) == 'undefined') {
    $("#no-camera-warning").removeClass('hidden');
    if (window.location.protocol == 'http:') {
      var https_url = "https://" + window.location.hostname + window.location.pathname;
      $("#no-camera-warning")
        .append("<p>You may need to switch to <a href='" +  https_url + "'>" + https_url + "</a></p>");
    }
  } else {
    navigator.mediaDevices.ondevicechange = function(event) {
      build_device_picker($("#device-picker"), /*include_remote*/false, on_device_selection);
    };
    $(window).resize(function() {
      on_device_selection($("#device-picker"));
    });
  }

  build_device_picker($("#device-picker"), /*include_remote*/false, on_device_selection);
});


function capture_photo(repo) {
  var photo_base_name = 'mobile-' + repo + '-' + g_barcode;

  var width = $("#preview").width();
  var height = $("#preview").height();
  var preview = $("#preview")[0];
  var stream = preview.srcObject;
  
  var canvas = document.createElement('canvas');
  canvas.width = width;
  canvas.height = height;
  canvas.getContext('2d').drawImage($("#preview")[0], 0, 0, width, height);
  var image_data_url = canvas.toDataURL('image/jpeg');
  canvas.remove();

  var byteString = atob(image_data_url.split(',')[1]);

  // write the bytes of the string to an ArrayBuffer
  var ab = new ArrayBuffer(byteString.length);
  var ia = new Uint8Array(ab);
  for (var i = 0; i < byteString.length; i++) {
    ia[i] = byteString.charCodeAt(i);
  }

  var blob = new Blob([ab], {type: 'image/jpeg'});

  // stuff into a form, so servers can easily receive it as a standard file upload
  var form_data = new FormData();
  form_data.append('action', 'photo.upload');
  form_data.append('barcode', g_barcode);
  form_data.append('repo', repo);
  form_data.append('photo', blob, photo_base_name + ".jpg");
  if (repo == 'head') {  // TODO
    form_data.append('autocrop', '1');
  }

  $.ajax('action.php',
         {type: 'POST',
          data: form_data,
          contentType: false,
          processData: false,
          success: function(data) {
            if (data.hasOwnProperty('thumbnail')) {
              update_thumbnail(repo, data['thumbnail']);
            }
          }
         });

  
  return false;
}


function update_thumbnail(repo, url) {
  $("#" + repo + "-button img.thumbnail").remove();
  $("<img/>").addClass('thumbnail').attr('src', url).appendTo($("#" + repo + "-button"));
}

function update_checked_in(passed) {
  $("#checked_in").prop('checked', passed);
  flipswitch_refresh("#checked_in");
}

function on_checkin_button() {
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'racer.pass',
                 barcode: g_barcode,
                 // New value will be opposite of what the control currently shows.
                 value: $("#checked_in").prop('checked') ? 0 : 1},
          success: function(json) {
            if (json.outcome.summary == 'success') {
              update_checked_in(!$("#checked_in").prop('checked'));
            } else {
              console.log(json);
              $("<pre/>").appendTo("body").css({
                position: 'fixed',
                top: 0,
                left: 0,
                height: '100vh',
                width: '100vh',
                'z-index': 100,
                background: 'white'}).text(JSON.stringify(json, null, 2));
            }
          }
         });
}

