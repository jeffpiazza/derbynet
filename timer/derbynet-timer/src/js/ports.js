
if ('serial' in navigator) {
  // This never seems to fire
  navigator.serial.addEventListener('connect', (e) => {
    // Connect to `e.target` or add it to a list of available ports.
    console.log('connect event', e);
    g_logger.internal_msg('serial connect event');
  });

  navigator.serial.addEventListener('disconnect', (e) => {
    // Remove `e.target` from the list of available ports.
    console.log('disconnect event', e);
    g_logger.internal_msg('serial disconnect event');
  });

  navigator.serial.getPorts().then((ports) => {
    // Initialize the list of available ports with `ports` on page load.
  });

  /*
    button.addEventListener('click', () => {
    const usbVendorId = ...;
    navigator.serial.requestPort({ filters: [{ usbVendorId }]}).then((port) => {
    // Connect to `port` or add it to the list of available ports.
    }).catch((e) => {
    // The user didn't select a port.
    });
    });

  */
}
