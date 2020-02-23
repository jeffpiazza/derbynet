package org.jeffpiazza.derby.gui;

import java.util.Vector;
import javax.swing.JList;
import javax.swing.ListModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import org.jeffpiazza.derby.devices.TimerTask;

public class SerialPortListController implements ListSelectionListener {
  private JList<SerialPortListElement> portList;
  private boolean userChoosing = true;  // Access via synchronized getter/setter
  private TimerTask timerTask;

  public SerialPortListController(JList<SerialPortListElement> portList) {
    this.portList = portList;
  }

  // Perform a new scan of serial ports and update the JList to reflect them.
  // There's a chance that the JList will differ from what the PortIterator
  // actually being used will enumerate, but that shouldn't be fatal, just
  // briefly confusing to see.
  public void updateSerialPorts(TimerTask timerTask, String[] portNames) {
    this.timerTask = timerTask;
    Vector<SerialPortListElement> portModel
        = new Vector<SerialPortListElement>(portNames.length);
    for (String p : portNames) {
      portModel.addElement(new SerialPortListElement(p));
    }
    portList.setListData(portModel);
  }

  public void setSerialPort(String portName) {
    ListModel<SerialPortListElement> model = portList.getModel();
    for (int i = 0; i < model.getSize(); ++i) {
      if (model.getElementAt(i).portName().equals(portName)) {
        setUserChoosing(false);
        try {
          portList.setSelectedIndex(i);
        } finally {
          setUserChoosing(true);
        }
        // We're about to try opening, so assume success
        portList.getSelectedValue().setWontOpen(false);
        return;
      }
    }
  }

  public void markSerialPortWontOpen() {
    SerialPortListElement selectedValue = portList.getSelectedValue();
    if (selectedValue != null) {
      selectedValue.setWontOpen(true);
    }
  }

  public void deselectAll() {
    setUserChoosing(false);
    try {
      portList.clearSelection();
    } finally {
      setUserChoosing(true);
    }
  }

  @Override
  public void valueChanged(ListSelectionEvent e) {
    if (isUserChoosing()) {
      SerialPortListElement v = portList.getSelectedValue();
      if (v != null) {
        timerTask.userChoosesSerialPort(v.portName());
      }
    }
  }

  private synchronized boolean isUserChoosing() {
    return userChoosing;
  }

  private synchronized void setUserChoosing(boolean choosing) {
    userChoosing = choosing;
  }
}
