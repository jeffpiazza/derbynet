// Barcode scanner imposes a pretty high CPU load, so can be turned off
// while debugging other issues.
var g_decode_barcodes = true;

var g_trouble_timeout = 0;
function report_trouble(msg) {
  clearTimeout(g_trouble_timeout);
  $("#trouble").empty().text(msg).removeClass('hidden');
  g_trouble_timeout = setTimeout(close_trouble, 6000);
}

function close_trouble() {
  $("#trouble").addClass('hidden');
}

var g_hover_timeout = 0;
function announce_hover(msg) {
  clearTimeout(g_hover_timeout);
  $("#announce-hover").css('display', 'block').removeClass('hidden').text(msg);
  g_hover_timeout = setTimeout(close_announce_hover, 500);
}
function close_announce_hover() {
  $("#announce-hover").fadeOut(500, function() {
    $("#announce-hover").addClass('hidden'); });
}

function show_camera() {
  build_device_picker($("#device-picker"), /*include_remote*/false, on_device_selection);
}

function using_landscape() {
  // This reports on the screen, not the viewport
  // return screen.availHeight < screen.availWidth;
  return window.innerHeight < window.innerWidth;
}

function before_show_slide_in() {
  if (using_landscape()) {
    $("#slide-in").css({'width': '0px',
                        'height': '100vh',
                        'display': 'none'});
  } else {
    $("#slide-in").css({'height': '0px',
                        'width': '100vw',
                        'display': 'none'});
  }
}
function show_slide_in(afterfn = false) {
  $("#slide-in").css('display', 'block');
  if (using_landscape()) {
    $("#controls-inner").css('margin-left', '0px');
    $("#slide-in").animate({width: $("#slide-in-inner").css('width')}, afterfn);
  } else {
    $("#controls-inner").css('margin-left',
                             ($("#controls").width() - $("#controls-inner").width()) / 2);
    $("#slide-in").animate({height: $("#slide-in-inner").css('height')}, afterfn);
  }
}
function hide_slide_in() {
  // If hiding the #slide-in div in response to a screen orientation change,
  // using_landscape() will report from the current orientation, not the one the
  // div was drawn for.
  var landscape = false;
  var offset = $("#slide-in").offset();
  if (offset.top == 0) {
    landscape = true;
  } else if (offset.left == 0) {
    landscape = false;
  } else {
    landscape = using_landscape();
  }

  if (landscape) {
    $("#slide-in").animate({width: '0px'},
                           function() {  $("#slide-in").css('display', 'none'); });
  } else {
    $("#slide-in").animate({height: '0px'},
                           function() {  $("#slide-in").css('display', 'none'); });
  }
}

function show_racer_list() {
  // The device-picker shows through #racer-list if it's not hidden, for reasons
  // I don't understand.
  $("#camera-div").addClass('hidden');
  $("#racer-list").show('slide', {direction: 'left'});
}
function hide_racer_list(afterfn = false) {
  $("#camera-div").removeClass('hidden');
  $("#racer-list").hide('slide', {direction: 'left'}, afterfn);
}

var codeReader = new ZXingBrowser.BrowserMultiFormatOneDReader();

var g_current_racer = {};
// One of:
//  - {} (empty) if no current racer is selected.
//  - { barcode: string }
//  - { racerid: racerid }


async function on_device_selection(selectq) {
  // If a stream is already open, stop it.
  stream = document.getElementById("preview").srcObject;
  if (stream != null) {
    stream.getTracks().forEach(function(track) {
      track.stop();
    });
  }	

  mobile_select_refresh($("#device-picker"));
  
  let device_id = selectq.find(':selected').val();
  let video_constraint = {deviceId: device_id,
                          // width: $("#preview").width(),
                          // height: $("#preview").height()
                         };

  navigator.mediaDevices.getUserMedia({video: video_constraint})
    .then(function(stream) {
      var tracks = stream.getTracks();
      if (tracks.length < 1) {
        report_trouble("No video streams available.");
        return;
      }

      var settings = tracks[0].getSettings();

      var max_height = $("#camera-div").height() - $("#device-picker-div").height();
      var aspect = settings.hasOwnProperty('aspectRatio') ? settings.aspectRatio
          : settings.width / settings.height;

      if (max_height >= $(window).width() / aspect) {
        console.log('Limiting video preview by width');
        // Display at max width in order not to be too tall
        $("#preview").css({'min-height': 0,
                           'height': 'auto',
                           'min-width': '100%',
                           'margin-left': 'auto'
                          });
      } else {
        console.log('Limiting video preview by HEIGHT');
        var width = max_height * aspect;
        $("#preview").css({'min-height': max_height,
                           'height': max_height,
                           'min-width': width + 'px',
                           'margin-left': ($("#camera-div").width() - width) / 2,
                          });
      }

      if (g_decode_barcodes) {
        codeReader.decodeFromStream(stream, 'preview',
                                    function(result, error, control) {
                                      if (result) {
                                        console.log("Recognized barcode:", result);
                                        on_recognized_barcode(result);
                                      } else if (error && error.getKind() != "NotFoundException") {
                                        report_trouble("ZXing error: " + error.getKind() + "\n\n" +
                                                       error.toString());
                                        console.log('error', error.getKind(), error);
                                      }
                                    });
      }
    });
}

function on_recognized_barcode(result) {
  if (g_current_racer['barcode'] == result.text) {
    return;
  }
  if (!result.text.startsWith("PWD")) {
    console.log("Rejecting false barcode ", result.text);
    return;
  }
  g_current_racer = {barcode: result.text};

  before_show_slide_in();

  $("#flash-overlay").css({display: 'block'}).fadeOut(800);
  $("#beep")[0].play();

  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'racer.list',
                 barcode: result.text
                },
          success: function(json) {
            set_current_racer(json);
            show_slide_in();
          }
         });
}

// json is the result of a racer.list query with arguments intended to narrow to
// a single racer.
function set_current_racer(json) {
  // firstname, lastname, carnumber, carname, note_from, headshot, car_photo,
  // classid, class, rankid, rank, passed, seq
  if (json['racers'].length == 0) {
    report_trouble('No racer identified');
  } else {
    var racer = json['racers'][0];
    $("#racer-div").text(racer.firstname + " " + racer.lastname +
                         "  " + racer.carnumber + " " + racer.carname);
    update_thumbnail('head', racer.headshot);
    update_checked_in(racer.passed);
    update_thumbnail('car',  racer.carphoto);
  }
}


function dismiss_greetings() {
  $("#greetings-background").addClass('hidden');
  show_camera();
}

var g_autocrop = true;
function on_autocrop_button_click() {
  g_autocrop = ! g_autocrop;
  $("#autocrop-button").toggleClass('passed', g_autocrop);
  announce_hover('Autocrop is ' + (g_autocrop ? "ON" : "OFF"));
}

function capture_photo(repo) {
  var photo_base_name = 'mobile-' + repo + '-' +
      // Only one of barcode and racerid will be populated
      (g_current_racer['barcode'] || '') + (g_current_racer['racerid'] || '');

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

  // stuff into a form, so server can receive it as a standard file upload
  var form_data = new FormData();
  form_data.append('action', 'photo.upload');
  if (g_current_racer['barcode']) {
    form_data.append('barcode', g_current_racer['barcode']);
  } else {
    form_data.append('racerid', g_current_racer['racerid']);
  }
  form_data.append('repo', repo);
  form_data.append('photo', blob, photo_base_name + ".jpg");
  if (g_autocrop && repo == 'head') {
    form_data.append('autocrop', '1');
  }
  console.log(form_data.keys());

  $.ajax('action.php',
         {type: 'POST',
          data: form_data,
          contentType: false,
          processData: false,
          success: function(data) {
            if (data.outcome.summary != 'success') {
              report_trouble('Upload ' + data.outcome.code + ': ' + data.outcome.description);
              console.log('Failed upload', data);
            } else if (data.hasOwnProperty('thumbnail')) {
              update_thumbnail(repo, data['thumbnail']);
            }
          }
         });

  
  return false;
}


function update_thumbnail(repo, url) {
  if (!url) {
    url = (repo == 'head' ? 'img/photo-headshot.png' : 'img/photo-car.png');
  }

  $("#" + repo + "-button img.thumbnail").remove();
  $("<img/>").addClass('thumbnail').attr('src', url).appendTo($("#" + repo + "-button div"));
}

function update_checked_in(passed) {
  $("#checkin-button").toggleClass('passed', passed);
  $("#checkin-button-text")
    .html(passed
          ? "Racer has<br/><span class='checked-in-status'>PASSED</span><br/>inspection"
          : "Racer has<br/><span class='checked-in-status'>NOT PASSED</span><br/>inspection");
}

function on_checkin_button_click() {
  var was_passed = $("#checkin-button").hasClass('passed');
  var form_data = {action: 'racer.pass',
                   // New value will be opposite of what the control currently shows.
                   value: was_passed ? 0 : 1};
  if (g_current_racer['barcode']) {
    form_data['barcode'] = g_current_racer['barcode'];
  } else {
    form_data['racer'] = g_current_racer['racerid'];
  }
  $.ajax('action.php',
         {type: 'POST',
          data: form_data,
          success: function(json) {
            if (json.outcome.summary == 'success') {
              update_checked_in(!was_passed);
              announce_hover((!was_passed)
                             ? "Racer has PASSED inspection"
                             : "Racer has NOT PASSED inspection");
            } else {
              console.log(json);
              report_trouble(JSON.stringify(json, null, 2));
            }
          }
         });
}

function refresh_racer_list() {
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'racer.list'},
          success: function(json) {
            $("#racer-list ul").empty();
            for (var ri = 0; ri < json['racers'].length; ++ri) {
              var r = json['racers'][ri];
              var entry = r['firstname'] + " " + r['lastname'];
              if (r['carnumber']) {
                entry += ', ' + r['carnumber'];
              }
              entry += ', ' + r['partition'];
              $("<li>").text(entry).attr('racerid', r['racerid']).appendTo("#racer-list ul")
                .on('click', function(event) { on_racer_chosen_from_list($(event.target).attr('racerid')); });
            }
          }});
}

function on_racer_chosen_from_list(racerid) {
  g_current_racer = {racerid: racerid};
  hide_racer_list();
  show_camera();
  before_show_slide_in();

  $("#flash-overlay").css({display: 'block'}).fadeOut(800);
  $("#beep")[0].play();

  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'racer.list',
                 racerid: racerid
                },
          success: function(json) {
            set_current_racer(json);
            show_slide_in();
          }
         });
}

function on_switch_to_camera_cllicked() {
  update_thumbnail('head', '');
  update_thumbnail('car', '');
  before_show_slide_in();

  hide_racer_list(function() {
    show_camera();
  });
}


function on_preview_click() {
  hide_slide_in();

  refresh_racer_list();
  show_racer_list();
}


// There may be multiple resize events too close together for visibility of
// #slide-in to work.
var g_restore_slidein_timeout = false;
function on_resize() {
  if ($("#camera-div").is(':visible')) {
    var slidein = (g_restore_slidein_timeout != false);
    console.log("on_resize sees g_restore_slidein_timeout", g_restore_slidein_timeout);
    clearTimeout(g_restore_slidein_timeout);
    if (!slidein && $("#slide-in").is(':visible')
        && $("#slide-in").width() > 0
        && $("#slide-in").height() > 0) {
      console.log("on_resize believes #slide_in is visible");
      slidein = true;
      hide_slide_in();
    }
    if (slidein) {
      g_restore_slidein_timeout = setTimeout(function() {
        console.log('Sliding in');
        before_show_slide_in();
        show_slide_in(function () { g_restore_slidein_timeout = false; });
      }, 500);
    }
    on_device_selection($("#device-picker"));
  }
}

$(function() {
  if (typeof(navigator.mediaDevices) == 'undefined') {
    $("#trouble").removeClass('hidden');
    if (window.location.protocol == 'http:') {
      var https_url = "https://" + window.location.hostname + window.location.pathname;
      // No timeout
      $("#trouble")
        .empty()
        .append("<p>No camera access is available.  " +
                "You may need to switch to <a href='" +  https_url + "'>" + https_url + "</a></p>");
    }
  } else {
    navigator.mediaDevices.ondevicechange = function(event) {
      build_device_picker($("#device-picker"), /*include_remote*/false, on_device_selection);
    };
    $(window).resize(on_resize);
  }
});
