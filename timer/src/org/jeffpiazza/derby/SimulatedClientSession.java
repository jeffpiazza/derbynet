package org.jeffpiazza.derby;

import java.io.IOException;
import java.net.URL;
import java.util.Random;
import org.w3c.dom.Element;

// For exercising a timer device class, this takes the place of a ClientSession
// to simulate the actions of a web host running races.
public class SimulatedClientSession extends ClientSession {
  private static int nlanes = 0;
  // Sent after sending a heat-ready, and precludes sending another heat-ready
  // until the current one is answered.
  private static String heatReadyString = null;

  public static void setNumberOfLanes(int n) {
    nlanes = n;
  }

  public SimulatedClientSession() {
    super("");
    random = new Random();
  }

  private Random random;

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
    boolean newHeatReady = false;
    boolean isStartedMessage = messageAndParams.contains("message=STARTED");
    if (messageAndParams.contains("message=FINISHED")) {
      // Receiving the finish results marks the end of the heat
      heatReadyString = null;
    }
    if (messageAndParams.contains("message=FINISHED")
        || messageAndParams.contains("message=HEARTBEAT")
        || messageAndParams.contains("message=IDENTIFIED")) {
      if (random.nextFloat() < 0.6) {
        newHeatReady = true;
      }
    }
    if (heatReadyString != null) {
      newHeatReady = false;
    }

    // TODO Some probability of an abort-heat message

    if (newHeatReady) {
      int laneMask = 0;
      for (int lane = 0; lane < nlanes; ++lane) {
        if (random.nextFloat() < 0.9) {
          laneMask |= (1 << lane);
        }
      }
      heatReadyString = "<heat-ready class=\"Simulated\""
          + " heat=\"" + (1 + random.nextInt(5)) + "\""
          + " lane-mask=\"" + laneMask + "\""
          + " round=\"1\" "
          + " roundid=\"" + (1 + random.nextInt(9)) + "\"/>\n";
    }

    if (heatReadyString != null && !isStartedMessage) {
      System.out.print("\t\t\t\t" + heatReadyString);
    }

    return parseResponse(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        + "<action-response action=\"timer-message\">\n"
        + "  <success/>\n"
        + (heatReadyString == null || isStartedMessage ? "" : heatReadyString)
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
