package org.jeffpiazza.derby;

import java.io.IOException;
import java.net.URL;
import java.util.Random;
import org.json.JSONObject;
import org.w3c.dom.Element;

// For exercising a timer device class, this takes the place of a ClientSession
// to simulate the actions of a web host running races.
public class SimulatedClientSession extends ClientSession {
  // Sent after sending a heat-ready, and precludes sending another heat-ready
  // until the current one is answered or aborted.
  private String heatReadyString = null;
  private int numberOfHeatsPrepared = 0;
  private Random random;

  public SimulatedClientSession() {
    super("");
    this.random = new Random();
  }

  @Override
  public JSONObject login(String role, String password) throws IOException {
    return new JSONObject("{\"outcome\": {\"summary\": \"successful\"}");
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
      int nlanes = Flag.lanes.value();
      int fullMask = (1 << nlanes) - 1;
      int laneMask = fullMask;

      // nlanes fully populated heat, then:
      //
      // for firstBye = 0 to n
      //   heat with just firstBye (1)
      //   for secondBye = firstBye + 1 to n
      //     heat with 2 byes
      //
      // Whole cycle is n(n-1)/2 + n + n heats,
      // or n(n+3)/2
      int cycle = nlanes * (nlanes + 3) / 2;
      int index = numberOfHeatsPrepared % cycle;
      if (index >= nlanes) {
        int k = nlanes - 1;
        for (int bye1 = 0; bye1 < nlanes; ++bye1) {
          laneMask = fullMask & ~(1 << bye1);
          ++k;
          if (index == k) {
            break;
          }
          for (int bye2 = bye1 + 1; bye2 < nlanes; ++bye2) {
            laneMask = fullMask & ~(1 << bye1) & ~(1 << bye2);
            ++k;
            if (index == k) {
              break;
            }
          }
          if (index == k) {
            break;
          }
        }
      }

      heatReadyString = "<heat-ready class=\"Simulated\""
          + " heat=\"" + (1 + random.nextInt(5)) + "\""
          + " lane-mask=\"" + laneMask + "\""
          + " round=\"1\""
          + " roundid=\"" + (1 + random.nextInt(9)) + "\"/>\n";

      LogWriter.simulationLog("Simulating heat-ready with " +
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
