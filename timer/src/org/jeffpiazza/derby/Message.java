package org.jeffpiazza.derby;

import java.io.*;
import java.net.URLEncoder;
import org.jeffpiazza.derby.devices.AllDeviceTypes;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.devices.TimerTask;

// TODO: Heartbeat should supply whatever ancillary information
// the timer supports (reset button pressed, lane blocked, etc.)
public interface Message {

  public String asParameters();

  // First contact to the server is a HELLO message.
  public static class Hello implements Message {
    public String asParameters() {
      System.out.println("   Sending HELLO");
      StringBuilder sb = new StringBuilder();
      sb.append("message=HELLO");
      sb.append("&interface=jar");
      sb.append("&build=")
          .append(Version.series())
          .append("-")
          .append(Version.revision());
      return sb.toString();
    }
  }

  // When scanning identifies the timer, send an IDENTIFIED message.  The
  // connection to the timer is then assumed OK until a MALFUNCTION message is
  // received.
  public static class Identified implements Message {
    private int nlanes;
    private String timer;
    private String humanName;
    private String identifier;
    private boolean confirmed;

    public Identified(int nlanes, String timer, String humanName,
                      String identifier, boolean confirmed) {
      this.nlanes = nlanes;
      this.timer = timer;
      this.humanName = humanName;
      this.identifier = identifier;
      this.confirmed = confirmed;
    }

    public String asParameters() {
      System.out.println("   Sending IDENTIFIED");
      StringBuilder sb = new StringBuilder();
      sb.append("message=IDENTIFIED");
      sb.append("&interface=jar");
      sb.append("&lane_count=").append(nlanes);
      sb.append("&timer=").append(timer);
      sb.append("&confirmed=").append(confirmed ? "1" : "0");
      if (humanName != null) {
        try {
          sb.append("&human=").append(URLEncoder.encode(humanName, "UTF-8"));
        } catch (UnsupportedEncodingException ex) {
        }
      }
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

    public LaneResult() {
    }

    public LaneResult(String time) {
      this.time = time;
    }

    public LaneResult(String time, int place) {
      this.time = time;
      this.place = place;
    }
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
          String time = results[i].time;
          if (time == null) {
            LogWriter.serial(
                "Lane " + (i + 1) + ": substituting 9.9999 for missing time");
            time = "9.9999";
          }
          sb.append("&lane").append(i + 1).append("=").append(time);
          if (results[i].place != 0 && !Flag.ignore_place.value()) {
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
    private int health;

    public Heartbeat(int health) {
      this.health = health;
    }

    public String asParameters() {
      return "message=HEARTBEAT&"
          + (health == TimerTask.UNHEALTHY ? "unhealthy"
             : health == TimerTask.HEALTHY ? "confirmed=1"
               : "confirmed=0");
    }
  }

  public static class Flags implements Message {
    public String asParameters() {
      StringBuilder sb = new StringBuilder("message=FLAGS");
      try {
        for (Flag flag : Flag.allFlags()) {
          if (flag.is_settable()) {
            sb.append("&flag-").append(flag.name()).append("=");
            sb.append(flag.typeName()).append(":");
            sb.append(URLEncoder.encode(flag.value() == null
                                        ? "null" : flag.value().toString(),
                                        "UTF-8"));
            sb.append("&desc-").append(flag.name()).append("=");
            sb.append(URLEncoder.encode("" + flag.description(), "UTF-8"));
          }
        }
        sb.append("&ports=");
        boolean first_port = true;
        for (String port : AllSerialPorts.getNames()) {
          sb.append(URLEncoder.encode((first_port ? "" : ",") + port, "UTF-8"));
          first_port = false;
        }
        for (Class<? extends TimerDevice> devclass
             : AllDeviceTypes.allTimerDeviceClasses()) {
          sb.append("&device-").append(devclass.getSimpleName()).append("=");
          sb.append(URLEncoder.encode(AllDeviceTypes.toHumanString(devclass),
                                      "UTF-8"));
        }
      } catch (UnsupportedEncodingException ex) {  // Won't happen
        return null;
      }

      return sb.toString();
    }
  }
}
