package org.jeffpiazza.derby.gui;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.ArrayList;
import javax.swing.*;
import org.jeffpiazza.derby.*;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.devices.TimerTask;

// On the HTTP side, the GUI makes this approximate progression:
//
// (obtain URL for web server)
// (contact web server and get roles)
// (choose role and password)
// (login and start HttpTask)
//
// Command-line arguments may let us skip some of these steps.
public class TimerGui implements RoleFinder.RoleFinderClient {
  private Components components;
  private Connector connector;
  private RoleFinder roleFinder;
  private boolean rolesPopulated = false;
  // If non-null, use this role for the first login attempt, even if it's not
  // one of the available choices
  private String requestedRole = null;

  private TimerClassListController timerClassListController;
  private SerialPortListController portListController;

  public TimerGui(Connector connector) {
    this.components = new Components();
    this.connector = connector;
    timerClassListController = new TimerClassListController(
        components.timerClassList);
    components.timerClassList.addListSelectionListener(timerClassListController);
    portListController = new SerialPortListController(components.portList);
    components.portList.addListSelectionListener(portListController);
  }

  public void addKeyListener(KeyListener k) { components.addKeyListener(k); }

  public void show() {
    components.setTitle("Derby Timer Management");
    components.getRootPane().setDefaultButton(components.connectButton);

    components.httpIconStatus.setIcon(new ImageIcon(getClass().getResource(
        "/status/trouble.png")));
    components.serialIconStatus.setIcon(new ImageIcon(getClass().
        getResource("/status/unknown.png")));

    components.portList.setCellRenderer(new SerialPortListRenderer());
    components.connectButton.addActionListener(new ActionListener() {
      @Override
      public void actionPerformed(ActionEvent e) {
        TimerGui.this.onConnectButtonClick();
      }
    });
    components.scanButton.addActionListener(new ActionListener() {
      @Override
      public void actionPerformed(ActionEvent e) {
        TimerGui.this.onScanButtonClick();
      }
    });
    components.showLogFileMenuItem.addActionListener(new ActionListener() {
      @Override
      public void actionPerformed(ActionEvent e) {
        LogWriter.showLogFile();
      }
    });

    components.pack();
    components.setVisible(true);
  }

  // If connection details were entered on the command line, use them to
  // prefill the corresponding GUI fields, and then treat as though user
  // performed equivalent interaction.  If all details provided, call
  // setUrl before setRoleAndPassword.
  public void setUrl(String url) {
    components.urlField.setText(url);
    onConnectButtonClick();
  }

  public void setRoleAndPassword(String role, String password) {
    components.roleComboBox.setSelectedItem(role);
    components.passwordField.setText(password);
    this.requestedRole = role;
  }

  // This is mainly used to introduce a SimulatedClientSession and skip all the
  // communication with an actual server.
  public void setClientSession(ClientSession session) {
    startHttpTask(session);
  }

  private synchronized RoleFinder getRoleFinder() {
    return roleFinder;
  }

  private synchronized void setRoleFinder(RoleFinder roleFinder) {
    this.roleFinder = roleFinder;
  }

  private synchronized boolean rolesPopulated() {
    return rolesPopulated;
  }

  private synchronized void setRolesPopulated(boolean rolesPopulated) {
    this.rolesPopulated = rolesPopulated;
    this.notifyAll();
  }

  private static Color green = new Color(52, 127, 79);
  private static Color black = Color.BLACK;
  private static Color red = Color.RED;
  private static Color defaultBackground = new Color(184, 207, 229);

  public void setHttpStatus(String message, Color color) {
    components.httpStatusLabel.setForeground(color);
    components.httpStatusLabel.setText(message);
  }

  // Status icons should be one of: "ok", "trouble", "unknown".
  // Check the build file for where these icons come from, and to change the
  // available choices.
  public static String icon_ok = "ok";
  public static String icon_trouble = "trouble";
  public static String icon_unknown = "unknown";

  public void setHttpStatus(String message, Color color, String icon) {
    setHttpStatus(message, color);
    components.httpIconStatus.setIcon(new ImageIcon(getClass().getResource(
        "/status/" + icon + ".png")));
  }

  public void setSerialStatus(String message) {
    components.serialStatusLabel.setText(message);
  }

  public void setSerialStatus(String message, Color color) {
    components.serialStatusLabel.setForeground(color);
    setSerialStatus(message);
  }

  public void setSerialStatus(String message, Color color, String icon) {
    setSerialStatus(message, color);
    components.serialIconStatus.setIcon(new ImageIcon(getClass().
        getResource("/status/" + icon + ".png")));
  }

  // Runs on dispatch thread; no individual invocation can be long-running.
  // The first click of the button will launch a RoleFinder in a separate
  // thread, which attempts to contact the server and obtain a list of valid
  // roles.  A second click is detectable by the existence of a RoleFinder
  // already created for the current url
  private void onConnectButtonClick() {
    RoleFinder roleFinder = getRoleFinder();
    if (roleFinder != null && !roleFinder.getServerAddress().equals(
        components.urlField.getText())) {
      // Cancel existing rolefinder
      roleFinder.cancel();
      setRoleFinder(null);
      roleFinder = null;
    }
    if (roleFinder == null) {
      setHttpStatus("Contacting server...", black, icon_unknown);
      components.roleComboBox.setEnabled(false);
      components.passwordField.setEnabled(false);
      startRoleFinder();
    } else {
       // There's an existing roleFinder for the current URL, and the user
      // clicked "Connect."  If we're still waiting for the roles to
      // populate, then ignore the button, otherwise start a login request by
      // startHttpTask.
      if (rolesPopulated()) {
        if (roleFinder != null) {
          startHttpTask(roleFinder.getSession());
        }
      } else {
        setHttpStatus("(Hold your horses)", black, icon_unknown);
      }
    }
  }

  // Start a separate thread to contact the server and ask it for the available
  // roles; use the results to populate the role picker.
  private void startRoleFinder() {
    setRoleFinder(RoleFinder.start(components.urlField.getText(), this));
    setRolesPopulated(false);
  }

  @Override
  public void rolesFound(ArrayList<String> roles, ClientSession session) {
    components.urlField.setText(session.getBaseUrl());
    for (String role : roles) {
      components.roleComboBox.addItem(role);
    }

    synchronized (this) {
      setRolesPopulated(true);
      // Try logging in to the first role with an empty password -- almost always
      // works, and makes for one less thing for the operator to have to do.
      onConnectButtonClick();
      setHttpStatus("Please log in", black, icon_unknown);
      components.roleComboBox.setEnabled(true);
      components.passwordField.setEnabled(true);
      components.passwordField.requestFocus();
    }
  }

  @Override
  public synchronized void roleFinderFailed(String message) {
    LogWriter.info("RoleFinder failed: " + message);
    setHttpStatus(message, red, icon_trouble);
    roleFinder = null;
  }

  private void startHttpTask(ClientSession clientSession) {
    String role = (String) components.roleComboBox.getSelectedItem();
    if (requestedRole != null) {
      // Setting the combobox may not have any effect if requestedRole doesn't
      // match one of the roles from the server, but try it for login anyway.
      components.roleComboBox.setSelectedItem(requestedRole);
      role = requestedRole;
      requestedRole = null;  // Only try it once
    }
    setHttpStatus("Logging in...", black, icon_unknown);
    HttpTask.start(clientSession, connector,
                   (String) components.roleComboBox.getSelectedItem(),
                   String.valueOf(components.passwordField.getPassword()),
                   new HttpTask.LoginCallback() {
                 @Override
                 public void onLoginSuccess() {
                   setHttpStatus("Connected", green, icon_ok);
                   // This may be useful if using a keyboard listener:
                   // components.requestFocus();
                 }

                 @Override
                 public void onLoginFailed(String message) {
                   setHttpStatus("Unsuccessful login", red, icon_trouble);
                 }
               });
  }

  public void updateSerialPorts(TimerTask timerTask, String[] portNames) {
    portListController.updateSerialPorts(timerTask, portNames);
  }

  public void setSerialPort(String portName) {
    portListController.setSerialPort(portName);
  }

  public void markSerialPortWontOpen() {
    portListController.markSerialPortWontOpen();
  }

  public void updateTimerClasses(TimerTask timerTask,
                                 Class<? extends TimerDevice>[] timerClasses) {
    timerClassListController.updateTimerClasses(timerTask, timerClasses);
  }

  public void setTimerClass(Class<? extends TimerDevice> timerClass) {
    timerClassListController.setTimerClass(timerClass);
  }

  private void onScanButtonClick() {
    System.out.println("Scan/Stop Scanning button not implemented");
  }

  public void confirmDevice(boolean confirmed) {
    components.portList.setSelectionBackground(green);
    components.timerClassList.setSelectionBackground(green);
    if (confirmed) {
      setSerialStatus("Timer device identified", green, icon_ok);
    } else {
      setSerialStatus("Timer device unconfirmed", red, icon_unknown);
    }
    // TODO components.scanButton.setVisible(false);
  }

  // Remove selections between scan cycles
  public void deselectAll() {
    portListController.deselectAll();
    components.portList.setSelectionBackground(defaultBackground);
    timerClassListController.deselectAll();
    components.timerClassList.setSelectionBackground(defaultBackground);
  }
}
