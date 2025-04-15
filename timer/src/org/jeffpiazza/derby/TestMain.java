package org.jeffpiazza.derby;

// Run some internal self-tests
//
// java -cp timer/dist/lib/derby-timer.jar org.jeffpiazza.derby.TestMain
//
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import jssc.SerialPortException;
import org.jeffpiazza.derby.devices.TimerDevice.RaceFinishedCallback;
import org.jeffpiazza.derby.devices.TimerResult;
import org.jeffpiazza.derby.profiles.NewBold;
import org.jeffpiazza.derby.profiles.TheJudge;
import org.jeffpiazza.derby.serialport.MockTimerPortWrapper;
import org.jeffpiazza.derby.serialport.TimerPortWrapper;
import org.jeffpiazza.derby.timer.Event;
import org.jeffpiazza.derby.timer.Profile;
import org.jeffpiazza.derby.timer.ProfileDetector;
import org.jeffpiazza.derby.timer.TimerDeviceWithProfile;

public class TestMain {
  public static String toString(Message.LaneResult[] results) {
    StringBuilder sb = new StringBuilder();
    sb.append("{");
    for (int i = 0; i < results.length; ++i) {
      if (i > 0) {
        sb.append(",");
      }
      sb.append(" ");
      if (results[i] == null) {
        sb.append("bye");
      } else {
        sb.append(results[i].time);
      }
    }
    sb.append(" }");
    return sb.toString();
  }

  private static boolean compare(Message.LaneResult[] results,
                                 Message.LaneResult[] expected) {
    if (results.length != expected.length) {
      System.err.println("  Got " + results.length
          + " lanes, expected " + expected.length);
      return false;
    }
    for (int i = 0; i < results.length; ++i) {
      if (results[i] != null) {
        if (!results[i].equals(expected[i])) {
          System.err.println("  Lane " + (i + 1) + ": got " + results[i]
              + ", expected " + expected[i]);
          return false;
        }
      }
    }
    return true;
  }

  public static boolean testTheJudge() throws SerialPortException {
    TimerDeviceWithProfile dev = new TheJudge(new MockTimerPortWrapper());
    Event.register(dev);

    ArrayList<ProfileDetector> detectors = new ArrayList<ProfileDetector>();
    for (Profile.Detector det : TheJudge.profile().matchers) {
      detectors.add(new ProfileDetector(det));
    }

    boolean[] finished = new boolean[]{false};
    synchronized (finished) {
      dev.registerRaceFinishedCallback(new RaceFinishedCallback() {
        @Override
        public void raceFinished(int roundid, int heat,
                                 Message.LaneResult[] results) {
          synchronized (finished) {
            Message msg = new Message.Finished(roundid, heat, results);
            Message.LaneResult[] expected = new Message.LaneResult[]{
              new Message.LaneResult(),
              null,
              new Message.LaneResult(),
              new Message.LaneResult(),
              new Message.LaneResult()};
            if (compare(results, expected)
                && msg.asParameters().equals(
                    "message=FINISHED&roundid=200&heat=1"
                    + "&lane1=9.9999&lane3=9.9999&lane4=9.9999&lane5=9.9999")) {
              finished[0] = true;
            }
            finished.notifyAll();
          }
        }
      });

      dev.prepareHeat(200, 1, 29);

      String line = "Race Over";
      for (ProfileDetector d : detectors) {
        String s2 = d.apply(line);
        if (s2 != line) {
          break;
        }
      }
      try {
        finished.wait(2000);
      } catch (InterruptedException ex) {
        Logger.getLogger(TestMain.class.getName()).log(Level.SEVERE, null, ex);
      }
      Event.unregister(dev);

      return finished[0];
    }
  }

  public static boolean testNewBoldDnf() throws SerialPortException {
    TimerDeviceWithProfile dev = new NewBold(new MockTimerPortWrapper());
    Event.register(dev);

    boolean[] finished = new boolean[]{false};
    synchronized (finished) {
      dev.registerRaceFinishedCallback(new RaceFinishedCallback() {
        @Override
        public void raceFinished(int roundid, int heat,
                                 Message.LaneResult[] results) {
          synchronized (finished) {
            Message msg = new Message.Finished(roundid, heat, results);
            Message.LaneResult[] expected = new Message.LaneResult[]{
              new Message.LaneResult("3.333"),
              null,
              new Message.LaneResult("1.111"),
              new Message.LaneResult(),
              new Message.LaneResult()};
            if (compare(results, expected)
                && msg.asParameters().equals(
                    "message=FINISHED&roundid=100&heat=1&lane1=3.333"
                    + "&lane3=1.111&lane4=9.9999&lane5=9.9999")) {
              finished[0] = true;
            }
            finished.notifyAll();
          }
        }
      });

      dev.prepareHeat(100, 1, 29);
      Event.send(Event.LANE_RESULT, new String[]{"3", "1.111"});
      Event.send(Event.LANE_RESULT, new String[]{"1", "3.333"});
      Event.send(Event.LANE_RESULT, new String[]{"0", "0.000"});
      Event.send(Event.LANE_RESULT, new String[]{"0", "0.000"});

      try {
        finished.wait(2000);
      } catch (InterruptedException ex) {
        Logger.getLogger(TestMain.class.getName()).log(Level.SEVERE, null, ex);
      }
    }
    Event.unregister(dev);
    return finished[0];
  }

  public static void main(String[] args) throws SerialPortException {
    if (!testTheJudge()) {
      System.err.println("testTheJudge fails");
      System.exit(1);
    }
    if (!testNewBoldDnf()) {
      System.err.println("testNewBoldDnf fails");
      System.exit(1);
    }

    System.err.println();
    System.err.println("All tests pass");
    System.exit(0);
  }
}
