package org.jeffpiazza.derby;

import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.file.Files;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.json.JSONObject;

public class ObsCallbacks implements TimerDevice.RaceStartedCallback,
                                     TimerDevice.RaceFinishedCallback {
  private ObsClient obsClient;

  public ObsCallbacks(Connector connector) {
    makeNewObsClient();
    connector.addRaceStartedCallback(this);
    connector.addRaceFinishedCallback(this);
  }

  private void makeNewObsClient() {
    if (Flag.obs_uri.value() != null && !Flag.obs_uri.value().isEmpty()) {
      System.err.println("Making OBS client to " + Flag.obs_uri.value());
      try {
        obsClient = new ObsClient(new URI(Flag.obs_uri.value()));
        obsClient.connect();
      } catch (URISyntaxException ex) {
        System.err.println("ObsCallbacks fails: " + ex);
      }
    }
  }

  private String obsMessageForFlag(String flag) {
    if (flag == null || flag.isEmpty()) {
      return null;
    }
    if (flag.charAt(0) == '@') {
      File f = new File(flag.substring(1));
      try {
        return new String(Files.readAllBytes(f.toPath()));
      } catch (IOException ex) {
        LogWriter.info("Unable to read message " + flag + ": "
            + ex.getMessage());
        return null;
      }
    }
    return new JSONObject()
        .put("request-type", "TriggerHotkeyBySequence")
        .put("message-id", "derby-timer-event")
        .put("keyId", flag)
        .toString();
  }

  @Override
  public void raceStarted() {
    if (obsClient != null && obsClient.isClosed()) {
      makeNewObsClient();
    }
    if (obsClient != null && obsClient.isOpen()) {
      String message = obsMessageForFlag(Flag.obs_start.value());
      if (message != null) {
        obsClient.send(message);
      }
    }
  }

  @Override
  public void raceFinished(int roundid, int heat, Message.LaneResult[] results) {
    if (obsClient != null && obsClient.isClosed()) {
      makeNewObsClient();
    }
    if (obsClient != null && obsClient.isOpen()) {
      String message = obsMessageForFlag(Flag.obs_finish.value());
      if (message != null) {
        obsClient.send(message);
      }
    }
  }
}
