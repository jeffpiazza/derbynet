package org.jeffpiazza.derby;

import java.io.IOException;
import java.net.URL;
import java.util.Random;
import org.w3c.dom.Element;

// For exercising a timer device class, this takes the place of a ClientSession
// to simulate the actions of a web host running races.
public class SimulatedClientSession extends ClientSession {
  // TODO That these are static implies there are multiple SimulatedClientSessions ?
  private static int nlanes = 0;
  // Sent after sending a heat-ready, and precludes sending another heat-ready
  // until the current one is answered or aborted.
  private String heatReadyString = null;
  private int numberOfHeatsPrepared = 0;
  private LogWriter logWriter;
  private Random random;

  public SimulatedClientSession(LogWriter logWriter) {
    super("");
    this.logWriter = logWriter;
    this.random = new Random();
  }

  public static void setNumberOfLanes(int n) {
    nlanes = n;
  }

  @Override
  public Element login(String username, String password) throws IOException {
    return parseResponse("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        + "<action-response action=\"login\" name=\"" + username + "\""
        + " password=\"...\">\n"
        + "<success>" + username + "</success>\n"
        + "</action-response>");
  }

  @Override
  public Element sendTimerMessage(String messageAndParams) throws IOException {
    System.out.println("\t\t\t" + messageAndParams.replace("&", " & "));
    boolean makeNewHeat = false;
    boolean isStartedMessage = messageAndParams.contains("message=STARTED");

    if (messageAndParams.contains("message=IDENTIFIED")) {
      // Do nothing on initial timer identification
      makeNewHeat = false;
      heatReadyString = null;
    } else if (messageAndParams.contains("message=HEARTBEAT")) {
      // Send a new heat if there's not already one pending
      makeNewHeat = (heatReadyString == null);
    } else if (messageAndParams.contains("message=FINISHED")) {
      // Receiving the finish results marks the end of the heat
      heatReadyString = null;
      makeNewHeat = true;
    }

    if (makeNewHeat) {
      int laneMask = 0;
      for (int lane = 0; lane < nlanes; ++lane) {
        laneMask |= (1 << lane);
      }

      int heats6 = numberOfHeatsPrepared % 6;
      if (heats6 >= 2) {
        int firstEmptyLane = random.nextInt(nlanes);
        laneMask &= ~(1 << firstEmptyLane);
        if (heats6 >= 4) {
          int secondEmptyLane = random.nextInt(nlanes);
          while (secondEmptyLane == firstEmptyLane) {
            secondEmptyLane = random.nextInt(nlanes);
          }
        laneMask &= ~(1 << secondEmptyLane);
        }
      }

      heatReadyString = "<heat-ready class=\"Simulated\""
          + " heat=\"" + (1 + random.nextInt(5)) + "\""
          + " lane-mask=\"" + laneMask + "\""
          + " round=\"1\""
          + " roundid=\"" + (1 + random.nextInt(9)) + "\"/>\n";

      logWriter.simulationLog("Simulating heat-ready with " +
          LogWriter.laneMaskString(laneMask, nlanes));
      System.out.print("\t\t\t\t" + heatReadyString);

      ++numberOfHeatsPrepared;
    }

    // The heatReadyString only gets sent the first time
    String additional = "";
    if (makeNewHeat) {
      additional = heatReadyString;
    }

    return parseResponse(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        + "<action-response action=\"timer-message\">\n"
        + "  <success/>\n"
        + additional
        + "</action-response>");
  }

  @Override
  // This is only used for "roles" query, and probably isn't called at all
  // when using a simulated client session.
  public Element doQuery(URL url) throws IOException {
    return parseResponse("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        + "<roles>\n"
        + "<role/>\n"
        + "<role timer_message=\"1\">Timer</role>\n"
        + "<role>Photo</role>\n"
        + "<role>RaceCrew</role>\n"
        + "<role timer_message=\"1\">RaceCoordinator</role>\n"
        + "</roles>");
  }
}
