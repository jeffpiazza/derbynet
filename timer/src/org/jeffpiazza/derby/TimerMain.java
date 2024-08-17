package org.jeffpiazza.derby;

import javax.swing.*;
import org.jeffpiazza.derby.gui.TimerGui;
import org.jeffpiazza.derby.devices.AllDeviceTypes;
import org.jeffpiazza.derby.devices.TimerTask;
import org.jeffpiazza.derby.serialport.SubprocessPortWrapper;

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
      LogWriter.stacktrace(t);
      System.exit(1);
    }
  }

  public static void main(String[] args) {
    RuntimeCondition.Initialize();
    String[] customArgs = Customizer.getCustomArgs();
    if (customArgs != null) {
      int consumed = Flag.parseCommandLineFlags(customArgs, 0);
      if (consumed != customArgs.length) {
        System.err.println("Only " + consumed + " customized args parse:");
        for (String a : customArgs) {
          System.err.println("  " + a);
        }
        System.exit(1);
      }
    }
    int consumed_args = Flag.parseCommandLineFlags(args, 0);

    if (Flag.version.value()) {
      System.out.println("DerbyNet version " + Version.get());
      System.exit(0);
    }

    String cmd_line_url = null;
    if (consumed_args < args.length && args[consumed_args].charAt(0) != '-') {
      cmd_line_url = args[consumed_args++];
    }

    if (consumed_args < args.length) {
      usage();
      System.exit(1);
    }

    makeLogWriter();

    String base_url = Customizer.getCustomUrl();  // Likely null
    if (base_url != null) {
      LogWriter.info("Custom URL: " + base_url);
    }
    // A URL on the command line takes precedence over URL from the jar
    if (cmd_line_url != null) {
      base_url = cmd_line_url;
      LogWriter.info("URL from the command line: " + base_url);
    }

    if (Flag.headless.value()) {
      if (base_url == null && !Flag.simulate_host.value()) {
        usage();
        System.err.println("**** URL is required for headless mode ****");
        System.exit(1);
      }
    }

    if (customArgs != null && customArgs.length > 0) {
      LogWriter.info("===== Arguments From Jar ==============");
      for (String arg : customArgs) {
        LogWriter.info(arg);
      }
      LogWriter.info("=======================================");
    }

    if (args.length > 0) {
      LogWriter.info("===== Command Line Arguments ==========");
      for (String arg : args) {
        LogWriter.info(arg);
      }
      LogWriter.info("=======================================");
    }

    Connector connector = new Connector();

    new ObsCallbacks(connector);

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
                       Flag.username.value(),
                       Flag.password.value(),
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

      // timerGui.addKeyListener(new KeyboardListener());

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
      if (!Flag.command.value().isEmpty()) {
        LogWriter.info("Spawning subprocess");
        SubprocessPortWrapper subprocessWrapper
            = new SubprocessPortWrapper(Flag.command.value());

        while (timerTask.device() == null) {
          timerTask.injectDevice(timerTask.tryOnePortWrapper(subprocessWrapper));
        }
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
    timerGui.setRoleAndPassword(Flag.username.value(),
                                Flag.password.value());
    if (base_url != null) {
      timerGui.setUrl(base_url);
    }
    if (simulatedSession != null) {
      timerGui.setClientSession(simulatedSession);
    }
    return timerGui;
  }
}
