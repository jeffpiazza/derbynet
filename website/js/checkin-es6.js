
// ********************************************************************************
// All ES6-only code falls below here, to allow (slightly reduced) functionality
// in pre-ES6 browsers (lots of otherwise functional iPads in this category).
// ********************************************************************************

// Re-writes the global g_cameras
async function enumerate_cameras() {
  g_cameras = new Array();
  await navigator.mediaDevices.enumerateDevices()
  .then(function(devices) {
    devices.forEach(function(device) {
      if (device.kind == "videoinput") {
        g_cameras.push(device.deviceId);
      }
    });
  });
}

async function handle_switch_camera() {
  await enumerate_cameras();
  g_cameraIndex++;
  if (g_cameraIndex >= g_cameras.length) {
    g_cameraIndex = 0;
  }

  Webcam.reset();
  setup_webcam();
  Webcam.attach('#preview');
}

show_photo_modal = async function(racerid, repo) {
  var firstname = $('#firstname-' + racerid).text();
  var lastname = $('#lastname-' + racerid).text();
  $("#racer_photo_name").text(firstname + ' ' + lastname);
  $("#racer_photo_repo").text(repo);
  $("#photo_modal_repo").val(repo);
  $("#photo_modal_racerid").val(racerid);

  set_autocrop_state(repo);

  // If the racer's already been checked in, don't offer "Capture & Check In" button
  $("#capture_and_check_in").toggleClass('hidden', $("#passed-" + racerid).prop('checked'));

  // TODO Two different submit buttons that set a global, g_check_in.  Eww.
  show_modal("#photo_modal", function() {
    preserve_autocrop_state();
    take_snapshot(racerid, repo, lastname + '-' + firstname);
      return false;
  });

  if (screen.width < screen.height) {
    g_width = 480;
    g_height = 640;
  }

  arm_webcam_dialog();

  await enumerate_cameras();
  Webcam.reset();
  setup_webcam();
  Webcam.attach('#preview');
};
