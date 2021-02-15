package org.jeffpiazza.derby.gui;

import java.util.Vector;
import javax.swing.JList;
import javax.swing.ListModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.devices.TimerTask;

// TODO For SerialPortListController, a user choice has to abort the inner
// (DeviceFinder) loop as well as fixing the PortIterator.  Need a "scan" object
// to maintain the state of the scan?
public class TimerClassListController implements ListSelectionListener {
  private JList<TimerClassListElement> timerClassList;
  // userChoosing tells valueChanged whether it's being called in response to
  // the user explicitly clicking, or just the scan updating the UI.
  private boolean userChoosing = true;  // Access via synchronized getter/setter
  private TimerTask timerTask;

  public TimerClassListController(JList<TimerClassListElement> timerClassList) {
    this.timerClassList = timerClassList;
  }

  public void updateTimerClasses(TimerTask timerTask,
                                 Class<? extends TimerDevice>[] timerClasses) {
    this.timerTask = timerTask;
    Vector<TimerClassListElement> timerElements
        = new Vector<TimerClassListElement>();
    for (Class<? extends TimerDevice> dev : timerClasses) {
      timerElements.addElement(new TimerClassListElement(dev));
    }
    timerClassList.setListData(timerElements);
  }

  public void setTimerClass(Class<? extends TimerDevice> timerClass) {
    ListModel<TimerClassListElement> model = timerClassList.getModel();
    if (timerClass == null) {
      timerClassList.clearSelection();
    } else {
      for (int i = 0; i < model.getSize(); ++i) {
        if (model.getElementAt(i).type() == timerClass) {
          setUserChoosing(false);
          try {
            timerClassList.setSelectedIndex(i);
          } finally {
            setUserChoosing(true);
          }
          return;
        }
      }
    }
  }

  public void deselectAll() {
    setUserChoosing(false);
    try {
      timerClassList.clearSelection();
    } finally {
      setUserChoosing(true);
    }
  }

  @Override
  public void valueChanged(ListSelectionEvent e) {
    if (isUserChoosing()) {
      TimerClassListElement v = timerClassList.getSelectedValue();
      if (v != null) {
        timerTask.userChoosesTimerClass(v.type());
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
