package org.jeffpiazza.derby.gui;

import java.awt.Color;
import java.awt.Component;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.ListCellRenderer;

class SerialPortListRenderer extends JLabel
    implements ListCellRenderer<SerialPortListElement> {
  public SerialPortListRenderer() {
    setOpaque(true);
  }

  @Override
  public Component getListCellRendererComponent(
      JList<? extends SerialPortListElement> list,
      SerialPortListElement value,
      int index, boolean isSelected,
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
    }
    return this;
  }

}
