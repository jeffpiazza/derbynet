'use strict';

// Incorporates a PortWrapper and the remote_start attribute of a profile.
class RemoteStart {
  port_wrapper;
  profile_remote_start;

  constructor(port_wrapper, profile_remote_start) {
    this.port_wrapper = port_wrapper;
    this.profile_remote_start = profile_remote_start;
  }

  has_remote_start() {
    return !this.profile_remote_start ||
      RuntimeCondition[this.profile_remote_start.has_remote_start]();
  }

  // If possible, trigger a gate release
  remote_start() {
    if (this.has_remote_start()) {
      this.port_wrapper.write(this.profile_remote_start.command);
      this.port_wrapper.drain();
      if (Flag.remote_start_starts_heat.value) {
        TimerEvent.send('RACE_STARTED');
      }
    }
  }
}


// NewDirections' SCI device (serial control interface?), and maybe other
// devices, can trigger a start gate based on the DTR line (pin 4).  
class DtrRemoteStart {
  port_wrapper;
  // DtrRemoteStart gets installed by flag, potentially replacing a RemoteStart
  // object.  Turning off the flag should restore that original RemoteStart, if
  // any.
  profile_remote_start;

  constructor(port_wrapper, profile_remote_start) {
    this.port_wrapper = port_wrapper;
    if (profile_remote_start instanceof RemoteStart) {
      this.profile_remote_start = profile_remote_start;
    }

    // Insurance: clear the DTR line
    this.port_wrapper.port.setSignals({ dataTerminalReady: false });
  }

  has_remote_start() {
    return true;
  }

  async remote_start() {
    await this.port_wrapper.port.setSignals({ dataTerminalReady: true });
    // Turn it back off after 500ms.
    await new Promise(r => setTimeout(r, 500));
    this.port_wrapper.port.setSignals({ dataTerminalReady: false });
    if (Flag.remote_start_starts_heat.value) {
      TimerEvent.send('RACE_STARTED');
    }
  }
}
