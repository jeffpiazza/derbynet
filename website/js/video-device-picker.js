
// cb gets called with the selected video stream.

function video_devices(curr_deviceId, cb) {
  let options = [];
  let found_current = false;
  navigator.mediaDevices.enumerateDevices()
    .then(devices => {
      devices.forEach(device => {
        if (device.kind == 'videoinput') {
          let opt = $("<option/>");
          opt.prop('value', device.deviceId)
            .text(device.label);
          if (curr_deviceId == device.deviceId) {
            opt.prop('selected', 'selected');
            found_current = true;
          }
          options.push(opt);
        }
      });
      cb(found_current, options);
    });
}
