package org.jeffpiazza.derby.timer;

import jssc.SerialPortException;
import org.jeffpiazza.derby.devices.RemoteStartInterface;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

public class ProfileRemoteStart implements RemoteStartInterface {
  private Profile.RemoteStart remote_start;
  private SerialPortWrapper portWrapper;

  public ProfileRemoteStart(SerialPortWrapper portWrapper,
                            Profile.RemoteStart remote_start) {
    this.portWrapper = portWrapper;
    this.remote_start = remote_start;
  }

  @Override
  public boolean hasRemoteStart() {
    return remote_start != null && remote_start.has_remote_start;
  }

  @Override
  public void remoteStart() throws SerialPortException {
    portWrapper.write(remote_start.command);
    portWrapper.drainForMs();
  }
}
