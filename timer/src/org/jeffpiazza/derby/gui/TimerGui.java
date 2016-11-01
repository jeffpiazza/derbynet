package org.jeffpiazza.derby.gui;

import jssc.SerialPort;
import org.jeffpiazza.derby.*;
import org.jeffpiazza.derby.devices.TimerDevice;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;

public class TimerGui {
  private Components components;
  private HttpTask.MessageTracer traceMessages;
  private HttpTask.MessageTracer traceHeartbeats;
  private Connector connector;
  private RoleFinder roleFinder;
  private boolean rolesPopulated = false;

  public TimerGui(HttpTask.MessageTracer traceMessages,
                  HttpTask.MessageTracer traceHeartbeats,
                  Connector connector) {
    this.components = new Components();
    this.connector = connector;
    this.traceMessages = traceMessages;
    this.traceHeartbeats = traceHeartbeats;
  }

  private static class PortListRenderer extends JLabel implements
      ListCellRenderer<SerialPortListElement> {
    public PortListRenderer() {
      setOpaque(true);
    }

    @Override
    public Component getListCellRendererComponent(
        JList<? extends SerialPortListElement> list,
        SerialPortListElement value,
        int index,
        boolean isSelected,
        boolean cellHasFocus) {

      setText(value.toString());

      if (isSelected) {
        setBackground(list.getSelectionBackground());
        setForeground(list.getSelectionForeground());
      } else {
        setBackground(list.getBackground());
        if (value.wontOpen()) {
          setForeground(Color.red);
        } else {
          setForeground(list.getForeground());
        }
      };

      return this;
    }
  }

  public void show() {
    components.setTitle("Derby Timer Management");
    components.getRootPane().setDefaultButton(components.connectButton);

    components.httpIconStatus.setIcon(new ImageIcon(getClass().getResource(
        "/status/trouble.png")));
    components.serialIconStatus.setIcon(new ImageIcon(getClass().
        getResource("/status/unknown.png")));

    components.portList.setCellRenderer(new PortListRenderer());
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

        onSecondConnectClick(getRoleFinder().getSession());
      }
    }).start();
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
      onFirstConnectClick();
    } else {
      onSecondConnectClick(roleFinder.getSession());
    }
  }

  private void onFirstConnectClick() {
    setHttpStatus("Contacting server...", black, icon_unknown);
    components.roleComboBox.setEnabled(false);
    components.passwordField.setEnabled(false);
    setRoleFinder(new RoleFinder(components.urlField.getText(), this));
    setRolesPopulated(false);
    (new Thread() {
      @Override
      public void run() {
        getRoleFinder().findRoles();
      }
    }).start();
  }

  private void onSecondConnectClick(ClientSession clientSession) {
    // There's an existing roleFinder for the current URL, and the user
    // clicked "Connect."  If we're still waiting for the roles to
    // populate, then ignore the button, otherwise start a login request
    if (rolesPopulated()) {
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
    } else {
      setHttpStatus("(Hold your horses)", black, icon_unknown);
    }
  }

  // Called once for each role to be added to the role combobox.  After the last role is added,
  // call rolesComplete()
  public void addRole(String role) {
    components.roleComboBox.addItem(role);
  }

  // Called to signify that all the appropriate roles from the server have been added to the role combobox
  public synchronized void rolesComplete() {
    setRolesPopulated(true);
    setHttpStatus("Please log in", black, icon_unknown);
    components.roleComboBox.setEnabled(true);
    components.passwordField.setEnabled(true);
    components.passwordField.requestFocus();
  }

  public synchronized void roleFinderFailed(String message) {
    setHttpStatus(message, red, icon_trouble);
    roleFinder = null;
  }

  // Perform a new scan of serial ports and update the JList to reflect them.  There's a chance that the JList will
  // differ from what the PortIterator actually being used will enumerate, but that shouldn't be fatal, just
  // briefly confusing to see.
  public void updateSerialPorts() {
    Vector<SerialPortListElement> portModel = new Vector<SerialPortListElement>();
    PortIterator portIterator = new PortIterator();
    while (portIterator.hasNext()) {
      portModel.addElement(new SerialPortListElement(portIterator.next()));
    }
    components.portList.setListData(portModel);
  }

  public void setSerialPort(SerialPort port) {
    ListModel<SerialPortListElement> model = components.portList.getModel();
    for (int i = 0; i < model.getSize(); ++i) {
      if (model.getElementAt(i).port().getPortName().equals(port.
          getPortName())) {
        components.portList.clearSelection();
        // components.portList.setSelectionForeground(Color.blue);
        components.portList.setSelectedIndex(i);
        // We're about to try opening, so assume success
        components.portList.getSelectedValue().setWontOpen(false);
        return;
      }
    }
  }

  public void markSerialPortWontOpen() {
    components.portList.getSelectedValue().setWontOpen(true);
  }

  public void initializeTimerClasses(DeviceFinder deviceFinder) {
    Vector<TimerClassListElement> timerModel
        = new Vector<TimerClassListElement>();
    for (Class<? extends TimerDevice> dev : deviceFinder.deviceClasses()) {
      timerModel.addElement(new TimerClassListElement(dev));
    }
    components.timerClassList.setListData(timerModel);
  }

  public void setTimerClass(Class<? extends TimerDevice> timerClass) {
    ListModel<TimerClassListElement> model = components.timerClassList.
        getModel();
    for (int i = 0; i < model.getSize(); ++i) {
      if (model.getElementAt(i).type() == timerClass) {
        components.timerClassList.clearSelection();
        // components.timerClassList.setSelectionForeground(Color.blue);
        components.timerClassList.setSelectedIndex(i);
        return;
      }
    }
  }

  private void onScanButtonClick() {
    System.out.println("Scan/Stop Scanning button not implemented");
  }

  public void confirmDevice(SerialPort port,
                            Class<? extends TimerDevice> timerClass) {
    components.portList.setSelectionBackground(green);
    components.timerClassList.setSelectionBackground(green);
    setSerialStatus("Timer device identified", green, icon_ok);
    // TODO components.scanButton.setVisible(false);
  }

  // Remove selections between scan cycles
  public void deselectAll() {
    components.portList.clearSelection();
    components.portList.setSelectionBackground(defaultBackground);
    components.timerClassList.clearSelection();
    components.timerClassList.setSelectionBackground(defaultBackground);
  }
}
