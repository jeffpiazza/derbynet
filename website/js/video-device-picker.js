
// cb gets called with the selected video stream.
function video_device_picker(cb) {
  let jq = $("<select id=\"device-picker\">");

  jq.on('input', function(event) {
    let device_id = $(event.target).find(':selected').val();
    navigator.mediaDevices.getUserMedia({ video: { deviceId: device_id } }).then(cb);
  });

  // Safari only lets you enumerate devices after the user grants permission, so
  // we effectively have to call getUserMedia before enumerateDevices.
  //
  // TODO Try enumerateDevices without getUserMedia first, and see if that
  // works.  BUT - picking the first entry will effectively ask for the user's
  // permission anyway, even if the user eventually wants to choose remote
  // camea.
  navigator.mediaDevices.getUserMedia({
    audio: false,
    video: true
  }).then(stream => {
    cb(stream);
    navigator.mediaDevices.enumerateDevices()
      .then(devices => {
        devices.forEach(device => {
          if (device.kind == 'videoinput') {
            jq.append("<option value=\""  + device.deviceId + "\">" + device.label + "</option>");
          }
        });
      });
  });

  // Likely returns before jq is actually populated, but eventually it will be.
  return jq;
}
