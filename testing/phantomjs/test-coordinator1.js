require('./preamble');

page.open(root + 'coordinator.php', function(status) {
  page.evaluate(function() { window.phantom_testing = true; });

  var poll_result = '<?xml version="1.0" encoding="UTF-8"?>\n' +
      '<coordinator_poll>\n' +
      '  <current-heat now-racing="1" use-master-sched="1" classid="2" roundid="2" round="1"' +
      '     tbodyid="1" heat="13" number-of-heats="13">White\'s Wolves</current-heat>\n' +
      '  <racer lane="1" name="Willard Woolfolk" carname="" carnumber="282"' +
      '     photo="" finishtime="3.977"/>\n' +
      '  <racer lane="2" name="Blake Burling" carname="" carnumber="207"' +
      '     photo="" finishtime="3.646"/>\n' +
      '  <racer lane="3" name="Elliot Eastman" carname="" carnumber="227"' +
      '     photo="" finishtime="2.295"/>\n' +
      '  <racer lane="4" name="Dexter Dawes" carname="" carnumber="222"' +
      '     photo="" finishtime="3.720"/>\n' +
      '  <timer-state lanes="4" last_contact="1496356697" state="1"' +
      '     icon="img/status/not_connected.png">NOT CONNECTED</timer-state>\n' +
      '  <replay-state last_contact="1496360776" state="1"' +
      '     icon="img/status/not_connected.png" connected="">NOT CONNECTED</replay-state>\n' +
      '  <round roundid="7" classid="2" class="White\'s Wolves" round="2" roster_size="3"' +
      '     passed="3" unscheduled="3" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"' +
      '     passed="17" unscheduled="0" heats_scheduled="17" heats_run="17"/>\n' +
      '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="17"' +
      '     passed="13" unscheduled="0" heats_scheduled="13" heats_run="13"/>\n' +
      '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="16"' +
      '     passed="2" unscheduled="0" heats_scheduled="4" heats_run="4"/>\n' +
      '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="16"' +
      '     passed="3" unscheduled="0" heats_scheduled="4" heats_run="4"/>\n' +
      '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"' +
      '     passed="0" unscheduled="0" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="6" classid="6" class="TheLastClass" round="1" roster_size="0"' +
      '     passed="0" unscheduled="0" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="8" classid="7" class="Grand Finals" round="1" roster_size="5"' +
      '     passed="5" unscheduled="5" heats_scheduled="0" heats_run="0"/>\n' +
      '</coordinator_poll>';

  page.evaluate(function(xml) { process_coordinator_poll_response((new DOMParser()).parseFromString(xml, 'text/xml')); },
                poll_result);

  assert.equal(page.evaluate(function() {
    return $(".scheduling_control_group .control_group.scheduling_control").length;
  }), 8);

  assert.includes("east", page.evaluate(function() {
    return $('#master-schedule-group .scheduling_control img').prop('src'); }));

  page.evaluate(function() { $('#master-schedule-group .scheduling_control').click(); });

  assert.includes("south", page.evaluate(function() {
    return $('#master-schedule-group .scheduling_control img').prop('src'); }));

  // A new poll response shouldn't close the control
  page.evaluate(function(xml) { process_coordinator_poll_response((new DOMParser()).parseFromString(xml, 'text/xml')); },
                poll_result);

  assert.includes("south", page.evaluate(function() {
    return $('#master-schedule-group .scheduling_control img').prop('src'); }));

  page.evaluate(function() { $('#master-schedule-group .scheduling_control').click(); });

  assert.includes("east", page.evaluate(function() {
    return $('#master-schedule-group .scheduling_control img').prop('src'); }));

  phantom.exit();
});

