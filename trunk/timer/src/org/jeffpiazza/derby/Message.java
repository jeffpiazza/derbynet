package org.jeffpiazza.derby;

import java.io.*;
import java.net.URLEncoder;

    // TODO: Heartbeat should supply whatever ancillary information
    // the timer supports (reset button pressed, lane blocked, etc.)

public interface Message {

    public String asParameters();

    public static class Hello implements Message {
        private int nlanes;
        public Hello(int nlanes) {
            this.nlanes = nlanes;
        }

        public String asParameters() {
            return "message=HELLO&nlanes=" + nlanes;
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
        private LaneResult[] results;
        public Finished(LaneResult[] results) {
            this.results = results;
        }

        public String asParameters() {
            StringBuilder sb = new StringBuilder();
            sb.append("message=FINISHED");
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
        private String errorMsg;
        public Malfunction(String errorMsg) {
            this.errorMsg = errorMsg;
        }

        public String asParameters() {
            try {
                return "message=MALFUNCTION&error=" + URLEncoder.encode(errorMsg, "UTF-8");
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
