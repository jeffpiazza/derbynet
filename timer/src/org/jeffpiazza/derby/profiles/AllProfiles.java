package org.jeffpiazza.derby.profiles;

// Prints all the timer profiles as JSON objects.
//
// Usage:
//
// java -cp timer/dist/lib/derby-timer.jar org.jeffpiazza.derby.profiles.AllProfiles
//
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.jeffpiazza.derby.Flag;
import org.jeffpiazza.derby.devices.AllDeviceTypes;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.timer.Profile;
import org.json.JSONArray;
import org.json.JSONStringer;

public class AllProfiles {
  public static Profile[] profiles() {
    ArrayList<Profile> profiles = new ArrayList<Profile>();
    for (Class<? extends TimerDevice> cl : AllDeviceTypes.allTimerDeviceClasses()) {
      try {
        Method m = cl.getMethod("profile");
        profiles.add((Profile) m.invoke(cl));
      } catch (Throwable ex) {
        Logger.getLogger(AllProfiles.class.getName()).log(Level.SEVERE, null, ex);
      }
    }
    return (Profile[]) profiles.toArray(new Profile[] {});
  }

  public static void main(String[] args) {
    Flag.beta_test.setValueText("true");
    JSONArray allProfiles = new JSONArray();
    for (Profile p : profiles()) {
      allProfiles.put(p.toJSON());
    }
    System.out.println(allProfiles.toString(2));
  }
}
