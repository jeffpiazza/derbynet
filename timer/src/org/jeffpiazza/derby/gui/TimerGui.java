package org.jeffpiazza.derby.gui;

import jssc.SerialPort;
import org.jeffpiazza.derby.*;
import org.jeffpiazza.derby.devices.TimerDevice;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import org.jeffpiazza.derby.devices.TimerTask;

// On the HTTP side, the GUI makes this approximate progression:
//
// (obtain URL for web server)
// (contact web server and get roles)
// (choose role and password)
// (login and start HttpTask)
//
// Command-line arguments may let us skip some of these steps.
public class TimerGui {
  private Components components;
  private Connector connector;
  private HttpTask.MessageTracer traceMessages;
  private HttpTask.MessageTracer traceHeartbeats;
  private LogWriter logwriter;
  private RoleFinder roleFinder;
  private boolean rolesPopulated = false;

  private TimerClassListController timerClassListController;
  private SerialPortListController portListController;

  public TimerGui(Connector connector, HttpTask.MessageTracer traceMessages,
                  HttpTask.MessageTracer traceHeartbeats, LogWriter logwriter) {
    this.components = new Components();
    this.connector = connector;
    this.traceMessages = traceMessages;
    this.traceHeartbeats = traceHeartbeats;
    this.logwriter = logwriter;
    timerClassListController = new TimerClassListController(
        components.timerClassList);
    components.timerClassList.addListSelectionListener(timerClassListController);
    portListController = new SerialPortListController(components.portList);
    components.portList.addListSelectionListener(portListController);
  }

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
        logwriter.showLogFile();
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

    (new Thread() {
      @Override
      public void run() {
        // Wait for RoleFinder to finish populating roles.  If there's no
        // RoleFinder, then give up (there's no URL, or the RoleFinder failed).
        while (!rolesPopulated()) {
          if (getRoleFinder() == null) {
            return;
          }
          synchronized (TimerGui.this) {
            try {
              TimerGui.this.wait();
            } catch (InterruptedException ex) {
            }
          }
        }

        startHttpTask(getRoleFinder().getSession());
      }
    }).start();
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
    }
    if (roleFinder == null) {
      setHttpStatus("Contacting server...", black, icon_unknown);
      components.roleComboBox.setEnabled(false);
      components.passwordField.setEnabled(false);
      startRoleFinder();
    } else // There's an existing roleFinder for the current URL, and the user
    // clicked "Connect."  If we're still waiting for the roles to
    // populate, then ignore the button, otherwise start a login request
    {
      if (rolesPopulated()) {
        startHttpTask(roleFinder.getSession());
      } else {
        setHttpStatus("(Hold your horses)", black, icon_unknown);
      }
    }
  }

  // Start a separate thread to contact the server and ask it for the available
  // roles; use the results to populate the role picker.
  private void startRoleFinder() {
    setRoleFinder(new RoleFinder(components.urlField.getText(), this));
    setRolesPopulated(false);
    (new Thread() {
      @Override
      public void run() {
        getRoleFinder().findRoles();
      }
    }).start();
  }

  private void startHttpTask(ClientSession clientSession) {
    setHttpStatus("Logging in...", black, icon_unknown);
    HttpTask.start(components.roleComboBox.getItemAt(
        components.roleComboBox.getSelectedIndex()),
                   new String(components.passwordField.getPassword()),
                   clientSession, traceMessages, traceHeartbeats,
                   connector,
                   new HttpTask.LoginCallback() {
                 @Override
                 public void onLoginSuccess() {
                   setHttpStatus("Connected", green, icon_ok);
                 }

                 @Override
                 public void onLoginFailed(String message) {
                   setHttpStatus("Unsuccessful login", red, icon_trouble);
                 }
               });
  }

  // Called once for each role to be added to the role combobox.  After the last role is added,
  // call rolesComplete()
  public void addRole(String role) {
    components.roleComboBox.addItem(role);
  }

  // Called to signify that all the appropriate roles from the server have
  // been added to the role combobox
  public synchronized void rolesComplete() {
    setRolesPopulated(true);
    // Try logging in to the first role with an empty password -- almost always
    // works, and makes for one less thing for the operator to have to do.
    onConnectButtonClick();
    setHttpStatus("Please log in", black, icon_unknown);
    components.roleComboBox.setEnabled(true);
    components.passwordField.setEnabled(true);
    components.passwordField.requestFocus();
  }

  public synchronized void roleFinderFailed(String message) {
    setHttpStatus(message, red, icon_trouble);
    roleFinder = null;
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

  public void confirmDevice() {
    components.portList.setSelectionBackground(green);
    components.timerClassList.setSelectionBackground(green);
    setSerialStatus("Timer device identified", green, icon_ok);
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
