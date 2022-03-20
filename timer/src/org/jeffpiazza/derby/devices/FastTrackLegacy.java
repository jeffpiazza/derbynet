package org.jeffpiazza.derby.devices;

import jssc.*;
import org.jeffpiazza.derby.Message;
import org.jeffpiazza.derby.serialport.SerialPortWrapper;

import java.util.regex.Matcher;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.LogWriter;

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
LR - Sets high until the switch is released, then low

When our automatic release gate is connected the laser gate bit is now used as a
command to release the cars.  So when you send the LR command the start gate will
begin its sequence to release the cars.

The preferred command for releasing the automatic release gate is with the LG
command, although LR could work as could LN.  All the commands were put in here so
that people could make their own release gates and software and make it work with
our cable.

*/
public class FastTrackLegacy extends TimerDeviceCommon {
  public FastTrackLegacy(SerialPortWrapper portWrapper) {
    super(portWrapper, null);
    gateWatcher = new GateWatcher(portWrapper) {
      // Interrogates the starting gate's state.  CAUTION: polling while a
      // race is running may cause the race results, sent asynchronously, to
      // be mixed with the RG response, making them unintelligible.
      @Override
      protected boolean interrogateGateIsClosed()
          throws NoResponseException, SerialPortException,
                 LostConnectionException {
        portWrapper.write(MicroWizard.READ_START_SWITCH);
        long deadline = System.currentTimeMillis() + 1000;
        String s;
        while ((s = portWrapper.next(deadline)) != null) {
          if (s.startsWith(MicroWizard.READ_START_SWITCH)) {
            if (s.length() >= 3) {
              return s.charAt(2) == '1';
            }
            // K1 timer seems to respond "RG" followed by separate "X"
            // response, but it doesn't otherwise appear to show the gate
            // state.
            // Per Stuart Ferguson (18 Jan 2019):
            // The "X" indicates that the option is disabled. We will need
            // the serial number of your timer to generate a code that will
            // enable this feature.  You can then purchase the unlock code for
            // $20 at http://microwizard.com/orderform.php
            s = portWrapper.next(deadline);
            if (s != null) {
              if (s.equals("X")) {
                setGateStateNotKnowable();
              }
            }
          }
        }
        throw new NoResponseException();
      }
    };

    // Once started, we expect a race result within 10 seconds; we allow an
    // extra second before considering the results overdue.
    rsm.setMaxRunningTimeLimit(11000);
  }

  public static String toHumanString() {
    return "FastTrack K-series";
  }

  public static final int MAX_LANES = 6;

  public boolean probe() throws SerialPortException {
    if (!portWrapper.setPortParams(SerialPort.BAUDRATE_9600,
                                   SerialPort.DATABITS_8,
                                   SerialPort.STOPBITS_1,
                                   SerialPort.PARITY_NONE)) {
      return false;
    }

    portWrapper.write(MicroWizard.READ_VERSION);

    // We're looking for a response that matches these:
    // Copyright (c) Micro Wizard 2002-2005
    // K3 Version 1.05A  Serial Number <nnnnn>
    //
    // Copyright (C) 2004 Micro Wizard
    // K1 Version 1.09D Serial Number <nnnnn>
    //
    // COPYRIGHT (c) MICRO WIZARD 2002
    // K2 Version 1.05a  Serial Number <nnnnn>
    long deadline = System.currentTimeMillis() + 2000;
    String s;
    while ((s = portWrapper.next(deadline)) != null) {
      if (s.indexOf("Micro Wizard") >= 0 || s.indexOf("MICRO WIZARD") >= 0) {
        s = portWrapper.next(deadline);
        if (s.startsWith("K")) {
          timerIdentifier = s;
          // Clean up the timer state and capture some details into the log
          portWrapper.
              writeAndDrainResponse(MicroWizard.RESET_ELIMINATOR_MODE, 2, 1000);
          portWrapper.writeAndDrainResponse(MicroWizard.NEW_FORMAT, 2, 1000);
          // A FastTrack timer that doesn't understand the N2 ("enhanced format")
          // command should just ignore it, but in case there are more severe
          // consequences, this variable lets the attempt be skipped.
          if (!Flag.skip_enhanced_format.value()) {
            portWrapper.writeAndDrainResponse(MicroWizard.ENHANCED_FORMAT, 2,
                                              1000);
          }
          MicroWizard.readFeatures(portWrapper);

          // This "RM" (Read Mode) command seems to silence the K1 timer.
          // TODO portWrapper.writeAndDrainResponse(READ_MODE);
          setUp();
          return true;
        }
      }
    }

    return false;
  }

  protected void setUp() {
    portWrapper.registerDetector(new SerialPortWrapper.Detector() {
      public String apply(String line) throws SerialPortException {
        Matcher m = TimerDeviceUtils.matchedCommonRaceResults(line);
        if (m != null) {
          Message.LaneResult[] results
              = TimerDeviceUtils.extractResults(line, m.start(), m.end(),
                                                MAX_LANES);
          raceFinished(results);
          return line.substring(0, m.start()) + line.substring(m.end());
        } else {
          return line;
        }
      }
    });

    MicroWizard.registerEarlyDetectorForReset(portWrapper);
  }

  @Override
  public int getSafeNumberOfLanes() {
    return MAX_LANES;
  }

  @Override
  protected void maskLanes(int lanemask) throws SerialPortException {
    // The CLEAR_LANE_MASK causes an "AC" response, but without a cr/lf to mark
    // a complete response.
    doMaskLanes(lanemask, MicroWizard.CLEAR_LANE_MASK, 0, MicroWizard.LANE_MASK,
                'A', 2);
    // Even without gate polling available, send a single reset command as
    // part of the preparation for the next heat
    if (!Flag.fasttrack_automatic_gate_release.value()) {
      portWrapper.writeAndDrainResponse(MicroWizard.RESET_LASER_GATE, 2, 500);
    }
  }

  private RemoteStartInterface remote_start = new RemoteStartInterface() {
    @Override
    public boolean hasRemoteStart() {
      return Flag.fasttrack_automatic_gate_release.value();
    }

    @Override
    public void remoteStart() throws SerialPortException {
      if (Flag.fasttrack_automatic_gate_release.value()) {
        LogWriter.serial("Sending remoteStart " + MicroWizard.PULSE_LASER_BIT);
        portWrapper.writeAndDrainResponse(MicroWizard.PULSE_LASER_BIT, 2, 500);
      }
    }
  };

  @Override
  public RemoteStartInterface getRemoteStart() {
    return remote_start;
  }

  // K2 timer may report DNFs as 0.000
  @Override
  protected void raceFinished(Message.LaneResult[] results)
      throws SerialPortException {
    super.raceFinished(TimerDeviceUtils.zeroesToNines(results));
  }

  public int getNumberOfLanes() throws SerialPortException {
    // FastTrack, at least older versions, doesn't report actual number of lanes.
    return 0;
  }

  @Override
  public void onTransition(RacingStateMachine.State oldState,
                           RacingStateMachine.State newState) {
    if (newState == RacingStateMachine.State.RESULTS_OVERDUE) {
      logOverdueResults();
    }
  }

  protected void whileInState(RacingStateMachine.State state)
      throws SerialPortException, LostConnectionException {
    if (state == RacingStateMachine.State.RESULTS_OVERDUE) {
      giveUpOnOverdueResults();
    } else if (state == RacingStateMachine.State.MARK && gateWatcher != null) {
      // prepareHeat() called; waiting for gate to close.
      // Detecting "gate closed" and getting to SET depends on
      // resetting the laser if a laser start gate; we do that continuously
      // until detecting the gate "closed" (laser hits sensor).
      //
      // If the gate state option is disabled, then we don't want to be
      // continuously resetting the laser gate, because we could receive results
      // at any instant, and they'd be disrupted by the reset dialog.
      if (System.currentTimeMillis() >= displayHoldUntilMillis())
        if (!Flag.fasttrack_automatic_gate_release.value()) {
          portWrapper.writeAndDrainResponse(MicroWizard.RESET_LASER_GATE, 2, 2000);
        }
      checkConnection();
    }
  }
}
