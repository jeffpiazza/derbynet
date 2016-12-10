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
    public Identified(int nlanes) {
      this.nlanes = nlanes;
    }

    public String asParameters() {
      System.out.println("   Sending IDENTIFIED");
      return "message=IDENTIFIED&lane_count=" + nlanes;
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
          sb.append("&lane").append(i + 1).append("=").append(results[i].time);
          if (results[i].place != 0) {
            sb.append("&place").append(i + 1).append("=").append(results[i].place);
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
