const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');

app.commandLine.appendSwitch('enable-features', 'ElectronSerialChooser');

// Handle creating/removing shortcuts on Windows when installing/uninstalling.
if (require('electron-squirrel-startup')) { // eslint-disable-line global-require
  app.quit();
}

const createWindow = async() => {
  // Create the browser window.
  const mainWindow = new BrowserWindow({
    width: 1600,
    height: 800,
    webPreferences: {
      // Apparently no longer needed:
      //   enableBlinkFeatures: 'Serial',
      // webSecurity: false
      nodeIntegration: true,
      contextIsolation: false,
    }
  });

  // The application's request_new_port method, when run as a standalone
  // application, will call requestPorts repeatedly until it throws.  The
  // 'select-serial-port' handler, below, will "choose" each available port in
  // turn until all have been chosen.  It uses this variable to keep track of
  // which ports have already been chosen.
  let requestedPorts = [];

  mainWindow.webContents.session.on('select-serial-port', (event, portList, webContents, callback) => {
    console.log('SELECT-SERIAL-PORT FIRED WITH', portList);
    event.preventDefault();
    
    let selectedPort = portList.find((device) => {      
      // Automatically pick a specific device instead of prompting user
      //return device.vendorId == 0x2341 && device.productId == 0x0043;

      // Automatically return the first usbserial device
      // return device.portName.includes("usbserial");
      if (requestedPorts.includes(device.portId)) {
        return false;
      }
      return true;
    });

    if (!selectedPort) {
      callback('')
    } else {
      requestedPorts.push(selectedPort.portId);
      callback(selectedPort.portId);
    }
  })

  /*
  mainWindow.webContents.session.on('serial-port-added', (event, port) => {
    console.log('serial-port-added FIRED WITH', port);
    event.preventDefault();
    mainWindow.webContents.send('serial-ports', 'serial-port-added');
  })

  mainWindow.webContents.session.on('serial-port-removed', (event, port) => {
    console.log('serial-port-removed FIRED WITH', port);
    event.preventDefault();
    mainWindow.webContents.send('serial-ports', 'serial-port-removed');
  })
  
  mainWindow.webContents.session.on('select-serial-port-cancelled', () => {
    console.log('select-serial-port-cancelled FIRED.');
    mainWindow.webContents.send('serial-ports', 'select-serial-port-cancelled');
  })
  */

  mainWindow.webContents.session.setPermissionCheckHandler(
    (webContents, permission, requestingOrigin, details) => {
      return permission === 'serial'
    });

  mainWindow.webContents.session.setDevicePermissionHandler((details) => {
    return details.deviceType === 'serial';
  });

  // and load the index.html of the app.
  await mainWindow.loadFile('src/timer.html');

  // Open the DevTools.
  mainWindow.webContents.openDevTools()

  mainWindow.webContents.executeJavaScript('on_scan_click()', /*userGesture*/true);
}


// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow);

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  // On OS X it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
