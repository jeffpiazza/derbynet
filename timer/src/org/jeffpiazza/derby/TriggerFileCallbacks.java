package org.jeffpiazza.derby;

import java.io.File;
import org.jeffpiazza.derby.devices.TimerDevice;

public class TriggerFileCallbacks implements TimerDevice.RaceStartedCallback,
                                             TimerDevice.RaceFinishedCallback {
  public TriggerFileCallbacks(Connector connector) {
    connector.addRaceStartedCallback(this);
    connector.addRaceFinishedCallback(this);
  }

  @Override
  public void raceStarted() {
    if (Flag.trigger_file_directory.value() != null
        && !Flag.trigger_file_directory.value().isEmpty()) {
      try {
        (new File(new File(Flag.trigger_file_directory.value()),
            "heat-started")).createNewFile();
      } catch (Throwable t) {
        LogWriter.info("Failed to create /tmp/heat-started: "
            + t.getMessage());
      }
    }
  }

  @Override
  public void raceFinished(int roundid, int heat, Message.LaneResult[] results) {
    if (Flag.trigger_file_directory.value() != null
        && !Flag.trigger_file_directory.value().isEmpty()) {
      try {
        (new File(new File(Flag.trigger_file_directory.value()),
            "heat-finished")).createNewFile();
      } catch (Throwable t) {
        LogWriter.info("Failed to create /tmp/heat-finished: "
            + t.getMessage());
      }
    }
  }
}
