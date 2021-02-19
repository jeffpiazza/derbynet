package org.jeffpiazza.derby;

import java.io.File;
import jssc.*;
import org.jeffpiazza.derby.devices.TimerDevice;
import org.jeffpiazza.derby.gui.TimerGui;

import javax.swing.*;
import org.jeffpiazza.derby.devices.AllDeviceTypes;
import org.jeffpiazza.derby.devices.RemoteStartInterface;
import org.jeffpiazza.derby.devices.TimerTask;

// Three threads for three "actors":
// timer polling loop runs on main thread,
// HttpTask runs on another thread,
// GUI event dispatch runs on a third thread.
public class TimerMain {
  public static void usage() {
    System.err.println("Usage: [options] <base-url>");
    Flag.usage();

    System.err.println("      Known devices:");
    AllDeviceTypes.listDeviceClassNames();
    System.err.println();
  }

  private static void makeLogWriter() {
    try {
      LogWriter.initialize();
      LogWriter.info("derby-timer.jar version " + Version.get());
      LogWriter.info("os.name = " + System.getProperty("os.name"));
    } catch (Throwable t) {
      t.printStackTrace();
      System.exit(1);
    }
  }

  public static void main(String[] args) {
    int consumed_args = Flag.parseCommandLineFlags(args, 0);

    if (Flag.version.value()) {
      System.out.println("DerbyNet version " + Version.get());
      System.exit(0);
    }

    String base_url = CustomUrlFinder.getCustomUrl();  // Likely null
    // A URL on the command line takes precedence over URL from the jar
    if (consumed_args < args.length && args[consumed_args].charAt(0) != '-') {
      base_url = args[consumed_args];
      ++consumed_args;
    }

    if (consumed_args < args.length) {
      usage();
      System.exit(1);
    }

    if (Flag.headless.value()) {
      if (base_url == null && !Flag.simulate_host.value()) {
        usage();
        System.exit(1);
      }
    }

    makeLogWriter();
    if (args.length > 0) {
      LogWriter.info("===== Command Line Arguments ==========");
      for (String arg : args) {
        LogWriter.info(arg);
      }
      LogWriter.info("=======================================");
    }

    Connector connector = new Connector();

    if (Flag.insecure.value()) {
      HttpsSecurityDisabler.disableHttpsSecurity();
    }

    SimulatedClientSession simulatedSession
        = Flag.simulate_host.value() ? new SimulatedClientSession() : null;
    try {
      TimerGui timerGui = null;
      if (!Flag.headless.value()) {
        timerGui = startTimerGui(connector, base_url, simulatedSession);
      } else {
        final ClientSession clientSession
            = simulatedSession == null ? new ClientSession(base_url)
              : simulatedSession;
        LogWriter.setClientSession(clientSession);
        HttpTask.start(clientSession, connector,
                       new HttpTask.LoginCallback() {
                     @Override
                     public void onLoginSuccess() {
                       System.err.println("Successful login");
                     }

                     @Override
                     public void onLoginFailed(String message) {
                       System.err.println("Unsuccessful login: " + message);
                       System.exit(1);
                     }
                   });
      }
      connector.setTimerGui(timerGui);

      TimerTask timerTask = new TimerTask(Flag.portname.value(),
                                          Flag.devicename.value(), timerGui,
                                          connector);
      if (Flag.simulate_timer.value()) {
        timerTask.setSimulatedTimer();
      }
      if (Flag.record.value()) {
        timerTask.setRecording();
      }
      if (Flag.playback.value() != null) {
        timerTask.setPlayback();
      }
      timerTask.run();
    } catch (Throwable t) {
      LogWriter.stacktrace(t);
    }
  }

  private static TimerGui startTimerGui(Connector connector,
                                        String base_url,
                                        ClientSession simulatedSession) {
    final TimerGui timerGui = new TimerGui(connector);
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        timerGui.show();
      }
    });
    if (base_url != null) {
      timerGui.setUrl(base_url);
    }
    timerGui.setRoleAndPassword(Flag.username.value(),
                                Flag.password.value());
    if (simulatedSession != null) {
      timerGui.setClientSession(simulatedSession);
    }
    return timerGui;
  }
}
