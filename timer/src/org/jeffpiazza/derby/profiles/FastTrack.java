package org.jeffpiazza.derby.profiles;

import org.jeffpiazza.derby.timer.Profile;
import jssc.SerialPort;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.StateMachine;

/*
From the Micro Wizard himself:

The FastTrack timer has 1 pin, 1 bit that is the laser gate bit.  This is
connected to a pin on the RJ-11 cable that goes to the start.

Our automatic release gate uses the same cable and so these commands can interact
with that gate as well.  However it uses the laser bit in a different way than the
laser gate.

LO - Sets that bit low
LN - Sets that bit high
LG - Sets high for 1 second, then low
LR - Sets high until the switch is released, then low  (RESET_LASER_GATE)

When our automatic release gate is connected the laser gate bit is now used as a
command to release the cars.  So when you send the LR command the start gate will
begin its sequence to release the cars.

The preferred command for releasing the automatic release gate is with the LG
command, although LR could work as could LN.  All the commands were put in here so
that people could make their own release gates and software and make it work with
our cable.
 */
public class FastTrack extends TimerDeviceWithProfile {
  public FastTrack(SerialPortWrapper portWrapper) {
    super(portWrapper, profile());
  }

  public static Profile profile() {
    Profile profile = Profile.forTimer("FastTrack K- or Q-series", "FastTrack-K")
        .params(SerialPort.BAUDRATE_9600,
                SerialPort.DATABITS_8,
                SerialPort.STOPBITS_1,
                SerialPort.PARITY_NONE)
        .max_lanes(6)
        // Copyright (c) Micro Wizard 2002-2005
        // K3 Version 1.05A  Serial Number <nnnnn>
        //
        // Copyright (C) 2004 Micro Wizard
        // K1 Version 1.09D Serial Number <nnnnn>
        //
        // COPYRIGHT (c) MICRO WIZARD 2002
        // K2 Version 1.05a  Serial Number <nnnnn>
        .prober("RV", "Micro Wizard|MICRO WIZARD", "^K|Model: Q")
        // RE: Reset eliminator mode
        // N1: "new" format
        // N2: enhanced format
        // RF: return features
        .setup(Flag.skip_enhanced_format.value()
               ? new String[]{"RE", "N1", "RF"}
               // Q1 timer also supports an "N3" mode, which allows timers longer
               // than 10 seconds.
               : new String[]{"RE", "N1", "N2", "RF"})
        .match(" *([A-Z])=(\\d+\\.\\d+)([^ ]?)", Event.LANE_RESULT, 1, 2)
        .heat_prep("MG", "M", 'A')
        .gate_watcher("RG" /* Read start switch condition */,
                      new Profile.Detector("RG0", Event.GATE_OPEN),
                      new Profile.Detector("RG1", Event.GATE_CLOSED),
                      // An "X" after RG means option disabled
                      new Profile.Detector("^X$",
                          Event.GATE_WATCHER_NOT_SUPPORTED))
        .remote_start(false, "LG");

    if (Flag.fasttrack_automatic_gate_release.value()) {
      profile.remote_start.has_remote_start = true;
    } else {
      profile.during(StateMachine.State.MARK, "LR");
    }

    return profile;
  }
}
