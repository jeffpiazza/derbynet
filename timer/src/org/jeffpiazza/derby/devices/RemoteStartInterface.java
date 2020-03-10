package org.jeffpiazza.derby.devices;

import jssc.SerialPortException;

public interface RemoteStartInterface {
  boolean hasRemoteStart();
  void remoteStart() throws SerialPortException;
}
