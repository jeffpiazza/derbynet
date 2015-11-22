package org.jeffpiazza.derby;

// TODO: Announce first failed attempt to contact server; announce first HELLO.
// TODO: Generate some ties

public class FakeTimerMain implements HttpTask.HeatReadyCallback, HttpTask.AbortHeatCallback {
  private int staging_time;
  private HttpTask task;

  public static void usage() {
    System.err.println("Usage: [-u <user>] [-p <password>] [-l <nlanes>] [-t] [-th] [-r] <base-url>");
    System.err.println("   -u, -p: Specify username and password for authenticating to web server");
    System.err.println("   -l: Specify number of lanes to report to web server");
    System.err.println("   -s: Staging time in seconds between heats");
    System.err.println("   -t: Trace non-heartbeat messages sent");
    System.err.println("   -th: Trace heartbeat messages sent");
    System.err.println("   -r: Show responses to traced messages");
  }

  public static void main(String[] args) {
    String username = "RaceCoordinator";
    String password = "doyourbest";
    int nlanes = 3;
    int staging_time = 10;
    StdoutMessageTrace traceMessages = null;
    StdoutMessageTrace traceHeartbeats = null;
    boolean traceResponses = false;

    int consumed_args = 0;
    while (consumed_args + 1 < args.length) {
      if (args[consumed_args].equals("-u") && consumed_args + 2 < args.length) {
        username = args[consumed_args + 1];
        consumed_args += 2;
      } else if (args[consumed_args].equals("-p") && consumed_args + 2 < args.length) {
        password = args[consumed_args + 1];
        consumed_args += 2;
      } else if (args[consumed_args].equals("-l") && consumed_args + 2 < args.length) {
        try {
          nlanes = Integer.parseInt(args[consumed_args + 1]);
        } catch (Throwable t) {
          t.printStackTrace();
        }
        consumed_args += 2;
      } else if (args[consumed_args].equals("-s")) {
        try {
          staging_time = Integer.parseInt(args[consumed_args + 1]);
        } catch (Throwable t) {
          t.printStackTrace();
        }
        consumed_args += 2;
      } else if (args[consumed_args].equals("-t")) {
        traceMessages = new StdoutMessageTrace();
        ++consumed_args;
      } else if (args[consumed_args].equals("-th")) {
        traceHeartbeats = new StdoutMessageTrace();
        ++consumed_args;
      } else if (args[consumed_args].equals("-r")) {
        traceResponses = true;
        ++consumed_args;
      } else {
        usage();
        System.exit(1);
      }
    }

    if (consumed_args + 1 != args.length) {
      usage();
      System.exit(1);
    }

    if (traceMessages != null) {
      traceMessages.traceResponses = traceResponses;
    }
    if (traceHeartbeats != null) {
      traceHeartbeats.traceResponses = traceResponses;
    }

    String base_url = args[consumed_args];

    System.out.println(Timestamp.string() + ": FakeTimerMain starts; base_url = " + base_url);

    try {
      ClientSession session = new ClientSession(base_url);
      if (ClientSession.wasSuccessful(session.login(username, password))) {
        (new FakeTimerMain(new HttpTask(session, traceMessages, traceHeartbeats),
                           staging_time))
        .runTest(nlanes);
      }
    } catch (Throwable t) {
      t.printStackTrace();
    }
  }

  private FakeTimerMain(HttpTask task, int staging_time) {
    this.task = task;
    this.staging_time = staging_time;
  }

  public class HeatRunner implements Runnable {
    private int staging_time;
    private int lanemask;
    public HeatRunner(int staging_time, int lanemask) {
      this.staging_time = staging_time;
      this.lanemask = lanemask;
    }

    public void run() {
      pause(staging_time);
      System.out.println(Timestamp.string() + ": Starting a new race");
      task.queueMessage(new Message.Started());
      pause(4);  // 4 seconds for a pretty slow race
      System.out.println(Timestamp.string() + ": Sending heat results");
      makeHeatResults(lanemask);
    }
  }

  public void heatReady(int lanemask) {
    System.out.println(Timestamp.string() + ": Received heat-ready message with lane mask " + lanemask);
    (new Thread(new HeatRunner(staging_time, lanemask))).start();
  }

  public void abortHeat() {
    System.out.println(Timestamp.string() + ": Received abort message.");
  }

  public void makeHeatResults(int lanemask) {
    int nlanes = 32 - Integer.numberOfLeadingZeros(lanemask);
    Message.LaneResult[] results = new Message.LaneResult[nlanes];

    for (int lane = 0; lanemask != 0; ++lane) {
      if ((lanemask & (1 << lane)) != 0) {
        results[lane] = new Message.LaneResult();
        results[lane].time = 2.0 + (Math.random() * 2.0) + "";
        lanemask ^= (1 << lane);
      }
    }

    task.queueMessage(new Message.Finished(results));
  }

  public void pause(int nsec) {
    try {
      Thread.sleep(nsec * 1000);
    } catch (Throwable t) {
      t.printStackTrace();
    }
  }

  public void runTest(int nlanes) {
    task.registerHeatReadyCallback(this);
    task.registerAbortHeatCallback(this);

    System.out.println(Timestamp.string() + ": Attempting HELLO message with " + nlanes + " lanes");
    task.queueMessage(new Message.Hello(nlanes));
    (new Thread(task)).start();
  }
}
