package org.jeffpiazza.derby;

import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

// TODO: Accept base_url from command line
// TODO: Announce first failed attempt to contact server; announce first HELLO.

public class FakeTimerMain implements HttpTask.HeatReadyCallback, HttpTask.AbortHeatCallback {
    private HttpTask task;

    public static void main(String[] args) {
        try {
            (new FakeTimerMain(new HttpTask("http://localhost/xsite", "RaceCoordinator", "doyourbest"))).runTest();
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    private FakeTimerMain(HttpTask task) {
        this.task = task;
    }

    public class HeatRunner implements Runnable {
        private int lanemask;
        public HeatRunner(int lanemask) {
            this.lanemask = lanemask;
        }

        public void run() {
            pause(6);
            System.out.println("Starting a new race");
            task.send(new Message.Started());
            pause(4);
            System.out.println("Sending heat results");
            makeHeatResults(lanemask);
        }
    }

    public void heatReady(int lanemask) {
        System.out.println("Received heat-ready message with lane mask " + lanemask);
        (new Thread(new HeatRunner(lanemask))).start();
    }

    public void abortHeat() {
        System.out.println("Received abort message.");
    }

    public void makeHeatResults(int lanemask) {
        Message.LaneResult[] results = new Message.LaneResult[4];

        for (int lane = 0; lanemask != 0; ++lane) {
            if ((lanemask & (1 << lane)) != 0) {
                results[lane] = new Message.LaneResult();
                results[lane].time = (Math.random() * 4.0) + "";
                lanemask ^= (1 << lane);
            }
        }

        task.send(new Message.Finished(results));
    }

    public void pause(int nsec) {
        try {
            Thread.sleep(nsec * 1000);
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    public void runTest() {
        task.registerHeatReadyCallback(this);
        task.registerAbortHeatCallback(this);

        task.send(new Message.Hello(3));
        (new Thread(task)).start();

        // After the initial Hello, task will only send a heartbeat every 30 seconds.
        pause(60);
    }
}
