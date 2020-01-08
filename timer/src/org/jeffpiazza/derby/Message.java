package org.jeffpiazza.derby;

import java.io.*;
import java.net.URLEncoder;

// TODO: Heartbeat should supply whatever ancillary information
// the timer supports (reset button pressed, lane blocked, etc.)
public interface Message {

  public String asParameters();

  // First contact to the server is a HELLO message.
  public static class Hello implements Message {
    public String asParameters() {
      System.out.println("   Sending HELLO");
      return "message=HELLO";
    }
  }

  // When scanning identifies the timer, send an IDENTIFIED message.  The
  // connection to the timer is then assumed OK until a MALFUNCTION message is
  // received.
  public static class Identified implements Message {
    private int nlanes;
    private String timer;
    private String identifier;

    public Identified(int nlanes, String timer, String identifier) {
      this.nlanes = nlanes;
      this.timer = timer;
      this.identifier = identifier;
    }

    public String asParameters() {
      System.out.println("   Sending IDENTIFIED");
      StringBuilder sb = new StringBuilder();
      sb.append("message=IDENTIFIED");
      sb.append("&lane_count=").append(nlanes);
      sb.append("&timer=").append(timer);
      if (identifier != null) {
        try {
          sb.append("&ident=").append(URLEncoder.encode(identifier, "UTF-8"));
        } catch (UnsupportedEncodingException ex) {
          // Won't happen, and we don't much care anyway
        }
      }

      return sb.toString();
    }
  }

  public static class Started implements Message {
    public String asParameters() {
      return "message=STARTED";
    }
  }

  public static class LaneResult {
    public String time;
    public int place;  // 0 if not known/stated
  }

  public static class Finished implements Message {
    private static boolean report_place_data = true;
    public static void ignorePlaceData() { report_place_data = false; }

    private int roundid;
    private int heat;
    private LaneResult[] results;

    public Finished(int roundid, int heat, LaneResult[] results) {
      this.roundid = roundid;
      this.heat = heat;
      this.results = results;
    }

    public String asParameters() {
      StringBuilder sb = new StringBuilder();
      sb.append("message=FINISHED");
      sb.append("&roundid=").append(roundid);
      sb.append("&heat=").append(heat);
      for (int i = 0; i < results.length; ++i) {
        if (results[i] != null) {
          String time = results[i].time;
          if (time == null) {
            time = "9.9999";
          }
          sb.append("&lane").append(i + 1).append("=").append(time);
          if (results[i].place != 0 && report_place_data) {
            sb.append("&place").append(i + 1).append("=").append(
                results[i].place);
          }
        }
      }
      return sb.toString();
    }
  }

  public static class Malfunction implements Message {
    private boolean detectable;
    private String errorMsg;

    public Malfunction(boolean detectable, String errorMsg) {
      this.detectable = detectable;
      this.errorMsg = errorMsg;
    }

    public String asParameters() {
      try {
        return "message=MALFUNCTION"
            + "&detectable=" + (detectable ? "1" : "0")
            + "&error=" + URLEncoder.encode(errorMsg, "UTF-8");
      } catch (UnsupportedEncodingException e) {  // Won't happen
        return null;
      }
    }
  }

  public static class Heartbeat implements Message {
    public String asParameters() {
      return "message=HEARTBEAT";  // TODO
    }
  }
}
