const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');

app.commandLine.appendSwitch('enable-features', 'ElectronSerialChooser');

// Handle creating/removing shortcuts on Windows when installing/uninstalling.
if (require('electron-squirrel-startup')) { // eslint-disable-line global-require
  app.quit();
}

const createWindow = () => {
  // Create the browser window.
  const mainWindow = new BrowserWindow({
    width: 1600,
    height: 600,
    webPreferences: {
      // Apparently no longer needed:
      //   enableBlinkFeatures: 'Serial',
      // webSecurity: false
      nodeIntegration: true,
      contextIsolation: false,
    }
  });

  console.log('createWindow method');

  mainWindow.webContents.session.on('select-serial-port', (event, portList, webContents, callback) => {
    console.log('SELECT-SERIAL-PORT FIRED WITH', portList);

    mainWindow.webContents.send('serial-ports', portList);
    console.log('Sent portList on serial-ports channel');

    // webContents.executeJavaScript(codestring, userGesture) -- Not good
    //
    // ipcRenderer.on(channel, listener)
    // ipcRenderer.send(channel, ...args)
    // ipcRenderer.invoke(channel, ...args) returns Promise for receiving a response
    // ipcRenderer.postMessage(channel, message) 

    /* SELECT-SERIAL-PORT FIRED WITH [
  {
    portId: '4A1BCFB62B8A6BD6ACECFBFC39279F82',
    portName: 'cu.Bluetooth-Incoming-Port'
  },
  {
    portId: 'EEB852387750551647DDAEDA1DAE7B07',
    portName: 'cu.usbserial-1410',
    displayName: 'USB-Serial Controller',
    vendorId: '1659',
    productId: '8963',
    usbDriverName: 'com.apple.DriverKit-AppleUSBPLCOM'
  }
*/
    //Display some type of dialog so that the user can pick a port
    /*dialog.showMessageBoxSync({
      ....
      ...
      ...
    });*/
    event.preventDefault();
    
    let selectedPort = portList.find((device) => {      
      // Automatically pick a specific device instead of prompting user
      //return device.vendorId == 0x2341 && device.productId == 0x0043;

      // Automatically return the first usbserial device
      return device.portName.includes("usbserial");
    });
    if (!selectedPort) {
      callback('')
    } else {
      callback(selectedPort.portId)
    }
  })

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

  // and load the index.html of the app.
  mainWindow.loadFile('src/timer.html')

  // Open the DevTools.
  mainWindow.webContents.openDevTools()
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

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and import them here.
