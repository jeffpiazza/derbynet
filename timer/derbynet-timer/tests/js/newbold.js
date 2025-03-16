'use strict';

function newBoldDnf() {
  // 29 = five lanes with lane 2 masked out
  var result = new HeatResult(29);
  console.assert(!result.isFilled());
  result.setLane(3, '1.111');
  result.setLane(1, '3.333');

  console.assert(!result.isFilled());
  // NewBold timer does this for two DNF lanes (4 and 5)
  result.setLane(0, '0.000');
  result.setLane(0, '0.000');
  console.assert(result.isFilled());

  // Event.send('RACE_FINISHED', [100, 1, result]);

  // TODO This writes the ajax message to the log, but we don't presently have a
  // way to test that it's what we expect:
  //
  // action: "timer-message"
  // lane1: "3.333"
  // lane3: "1.111"
  // lane4: "9.9999"
  // lane5: "9.9999"
  // message: "FINISHED"
  // remote-start: "NO"
  g_host_poller.onEvent('RACE_FINISHED', [100, 1, result]);
}


function all_lanes() {
  var proxy = new TimerProxy({}, {});
  proxy.result = new HeatResult(29);
  TimerEvent.register_unique(proxy);
  TimerEvent.send('LANE_RESULT', ['3', '1.111']);
  TimerEvent.send('LANE_RESULT', ['1', '3.333']);
  TimerEvent.send('LANE_RESULT', ['5', '5.555']);
  TimerEvent.send('LANE_RESULT', ['4', '4.444']);
  // TODO Should trigger a RACE_FINISHED event that causes a FINISHED message to
  // be sent to the server.
}
