package org.jeffpiazza.derby.gui;

import jssc.SerialPort;
import org.jeffpiazza.derby.*;
import org.jeffpiazza.derby.devices.TimerDevice;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

public class TimerGui {
  private Components components;
  private HttpTask.MessageTracer traceMessages;
  private HttpTask.MessageTracer traceHeartbeats;
  private Connector connector;
  private RoleFinder roleFinder;
  private boolean rolesPopulated = false;

  public TimerGui(HttpTask.MessageTracer traceMessages, HttpTask.MessageTracer traceHeartbeats, Connector connector) {
    this.components = new Components();
    this.connector = connector;
    this.traceMessages = traceMessages;
    this.traceHeartbeats = traceHeartbeats;
  }

  public void show() {
    components.setTitle("Derby Timer Management");
    components.getRootPane().setDefaultButton(components.connectButton);

    components.httpIconStatus.setIcon(new ImageIcon(getClass().getResource("/status_trouble.png")));
    components.serialIconStatus.setIcon(new ImageIcon(getClass().getResource("/status_unknown.png")));

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

  private synchronized RoleFinder getRoleFinder() { return roleFinder; }
  private void setRoleFinder(RoleFinder roleFinder) { this.roleFinder = roleFinder; }

  private synchronized boolean rolesPopulated() {
    return rolesPopulated;
  }

  private static Color green = new Color(52, 127, 79);
  private static Color black = Color.BLACK;
  private static Color red = Color.RED;

  private void setHttpStatus(String message, Color color) {
    components.httpStatusLabel.setForeground(color);
    components.httpStatusLabel.setText(message);
  }

  // icon should be one of: "ok", "trouble", "unknown"
  private void setHttpStatus(String message, Color color, String icon) {
    setHttpStatus(message, color);
    components.httpIconStatus.setIcon(new ImageIcon(getClass().getResource("/status_" + icon + ".png")));
  }

  private void setSerialStatus(String message, Color color) {
    components.serialStatusLabel.setForeground(color);
    components.serialStatusLabel.setText(message);
  }

  private void setSerialStatus(String message, Color color, String icon) {
    setSerialStatus(message, color);
    components.serialIconStatus.setIcon(new ImageIcon(getClass().getResource("/status_" + icon + ".png")));
  }

  // Runs on dispatch thread; no individual invocation can be long-running
  private void onConnectButtonClick() {
    RoleFinder roleFinder = getRoleFinder();
    if (roleFinder != null && !roleFinder.getServerAddress().equals(components.urlField.getText())) {
      // Cancel existing rolefinder
      roleFinder.cancel();
      setRoleFinder(null);
    }
    if (roleFinder == null) {
      setHttpStatus("Contacting server...", black, "unknown");
      components.roleComboBox.setEnabled(false);
      components.passwordField.setEnabled(false);
      setRoleFinder(new RoleFinder(components.urlField.getText(), this));
      rolesPopulated = false;  // TODO synchronized?
      (new Thread() {
        @Override
        public void run() {
          getRoleFinder().findRoles();
        }
      }).start();
    } else {
      // There's an existing roleFinder for the current URL, and the user clicked "Connect."  If we're still waiting
      // for the roles to populate, then ignore the button, otherwise start a login request
      if (rolesPopulated()) {
        setHttpStatus("Logging in...", black, "unknown");
        HttpTask.start(components.roleComboBox.getItemAt(
            components.roleComboBox.getSelectedIndex()),
            new String(components.passwordField.getPassword()),
            roleFinder.getSession(),
            traceMessages, traceHeartbeats, connector, new HttpTask.LoginCallback() {
              @Override
              public void onLoginSuccess() {
                setHttpStatus("Connected", green, "ok");
              }

              @Override
              public void onLoginFailed(String message) {
                setHttpStatus("Unsuccessful login", red, "trouble");
              }
            });
      } else {
        setHttpStatus("(Hold your horses)", black, "unknown");
      }
    }
  }

  // Called once for each role to be added to the role combobox.  After the last role is added,
  // call rolesComplete()
  public void addRole(String role) {
    components.roleComboBox.addItem(role);
  }

  // Called to signify that all the appropriate roles from the server have been added to the role combobox
  public synchronized void rolesComplete() {
    rolesPopulated = true;
    setHttpStatus("Please log in", black, "unknown");
    components.roleComboBox.setEnabled(true);
    components.passwordField.setEnabled(true);
    components.passwordField.requestFocus();
  }

  public synchronized void roleFinderFailed(String message) {
    setHttpStatus(message, red, "trouble");
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
      if (model.getElementAt(i).port().getPortName().equals(port.getPortName())) {
        components.portList.clearSelection();
        components.portList.setSelectionBackground(Color.blue);
        components.portList.setSelectedIndex(i);
        return;
      }
    }
  }

  public void initializeTimerClasses(DeviceFinder deviceFinder) {
    Vector<TimerClassListElement> timerModel =
        new Vector<TimerClassListElement>();
    for (Class<? extends TimerDevice> dev : deviceFinder.deviceClasses()) {
      timerModel.addElement(new TimerClassListElement(dev));
    }
    components.timerClassList.setListData(timerModel);
  }

  public void setTimerClass(Class<? extends TimerDevice> timerClass) {
    ListModel<TimerClassListElement> model = components.timerClassList.getModel();
    for (int i = 0; i < model.getSize(); ++i) {
      if (model.getElementAt(i).type() == timerClass) {
        components.timerClassList.clearSelection();
        components.timerClassList.setSelectionBackground(Color.blue);
        components.timerClassList.setSelectedIndex(i);
        return;
      }
    }
  }

  private void onScanButtonClick() {
    System.out.println("Scan/Stop Scanning button not implemented");
  }

  public static class SelectedListCellRenderer extends DefaultListCellRenderer {
    private Color color;
    SelectedListCellRenderer(Color color) { this.color = color; }

    @Override
    public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
      Component c = super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
      if (isSelected) {
        c.setBackground(color);
      }
      return c;
    }
  }

  public void confirmDevice(SerialPort port, Class<? extends TimerDevice> timerClass) {
    components.portList.setCellRenderer(new SelectedListCellRenderer(green));
    components.timerClassList.setCellRenderer(new SelectedListCellRenderer(green));
    setSerialStatus("Timer device identified", green, "ok");
    // TODO components.scanButton.setVisible(false);
  }

  // Remove selections between scan cycles
  public void deselectAll() {
    components.portList.clearSelection();
    components.timerClassList.clearSelection();
  }
}
