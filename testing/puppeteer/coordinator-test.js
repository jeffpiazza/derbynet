const assert = require('./assert.js');

const puppeteer = require('puppeteer');

var root = 'http://localhost/derbynet';
if (process.argv.length > 2) {
  root = process.argv[2];
}
if (root.substring(0, 'http://'.length) != 'http://') {
  root = 'http://' + root;
}
if (root.substring(root.length - 1) == '/') {
  root = root.substring(0, root.length - 1);
}

var debugging = false

var role = 'RaceCoordinator';
var password = 'doyourbest';

puppeteer.launch({devtools: debugging}).then(async browser => {
  const all_pages = await browser.pages();
  const page = all_pages[0];
  page.setViewport({width: 1200,
                    height: 1800});
// TODO  page.on('console', msg => { console.log(msg); });

  await page.goto(root + '/login.php');
  await page.evaluate(function (role, password) {
    window.handle_login(role, password);
  }, role, password);
  await page.waitForNavigation();

  await page.evaluateOnNewDocument(() => {
    window.phantom_testing = true;
  });

  await page.goto(root + '/coordinator.php');

  await page.evaluate(() => {
    $.ajax = async function (url, config) {
      var xml = await window.onAjax(url, config); 
      if (xml) {
        config.success((new DOMParser()).parseFromString(xml, 'text/xml'));
      }
    };
  });

  async function open_scheduling_control_group(selector) {
    await page.evaluate((selector) => { $(selector).click(); }, selector);
    
    // Wait for control group to open
    await page.waitFor((selector) => { return $(selector).height() > 200; }, {}, selector);
  }

  // =================================================== First simulated poll =====================================
  await page.evaluate(function(xml) { process_coordinator_poll_response((new DOMParser()).parseFromString(xml, 'text/xml')); },
                '<?xml version="1.0" encoding="UTF-8"?>\n' +
                '<coordinator_poll>\n' +
                '  <current-heat now-racing="1" use-master-sched="0" classid="3" roundid="3" round="1"' +
                '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
                '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime=""/>\n' +
                '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime=""/>\n' +
                '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime=""/>\n' +
                '  <timer-state lanes="4" last_contact="1526246081" state="3"\n' +
                '         icon="img/status/ok.png">Staging</timer-state>\n' +
                '  <replay-state last_contact="0" state="1" icon="img/status/not_connected.png"\n' +
                '         connected="">NOT CONNECTED</replay-state>\n' +
                '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"\n' +
                '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
                '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="18"\n' +
                '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
                '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="17"\n' +
                '         passed="3" unscheduled="0" heats_scheduled="4" heats_run="2"/>\n' +
                '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="15"\n' +
                '         passed="2" unscheduled="2" heats_scheduled="0" heats_run="0"/>\n' +
                '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"\n' +
                '         passed="5" unscheduled="5" heats_scheduled="0" heats_run="0"/>\n' +
                '  <round roundid="7" classid="7" class="TheLastClass" round="1" roster_size="0"\n' +
                '         passed="0" unscheduled="0" heats_scheduled="0" heats_run="0"/>\n' +
                      '</coordinator_poll>');

  assert.equal({"now-racing":[3],
                "ready-to-race":[],
                "not-yet-scheduled":[4,5,7],
                "done-racing":[1,2],
               },
               await page.evaluate(() => { return g_rounds_layout; }));

  assert.equal(6, await page.$$eval(".scheduling_control_group .control_group.scheduling_control",
                                    controls => { return controls.length; }));

  assert.equal([[],    // Headers row
                ["1","Juan Jacobsen","343",""],
                ["2","Jeffress Jamison","139",""],
                ["3","Antoine Akiyama","303",""]],
               await page.evaluate(function() {
                 var table = [];
                 $("#now-racing-group .heat-lineup table tr").each(function(index, tr) {
                   var row = [];
                   $(tr).find("td").each(function(index, td) {
                     row.push($(td).text());
                   });
                   table.push(row);
                 });
                 return table;
               }));

  assert.equal("1", await page.$eval("#now-racing-group .roundno", span => { return $(span).text(); }));
  assert.equal("Bears and Frèr",
               await page.$eval("#now-racing-group > div > h3", h3 => { return $(h3).text(); }));
  assert.equal("17 racer(s), 3 passed, 3 in schedule.4 heats scheduled, 2 run.",
               await page.$eval("#now-racing-group p", p => { return $(p).text(); }));

  assert.equal([
    // Now racing
    "3", 0,
    // Not yet scheduled
    "4", 1, "Schedule",
    "5", 1, "Schedule",
    "7", 0,  // No passed racers
    // Done racing
    "1", 1, "Make Changes",
    "2", 1, "Make Changes"],
               await page.evaluate(function() {
                 var buttons = [];
                 $("div.control_group[data-roundid]").each(function() {
                   buttons.push($(this).attr('data-roundid'));
                   buttons.push($(this).find("input[type='button']").length);
                   $(this).find("input[type='button']").each(function() {
                     buttons.push($(this).val());
                   });
                 });
                 return buttons;
               }));

  assert.equal(["Add New Rounds"],
               await page.evaluate(function() {
                 var buttons = [];
                 $("#supplemental-control-group input[type='button']").each(function() {
                   buttons.push($(this).val());
                 });
                 return buttons;
               }));

  // ===============================  =====================================

  assert.equal("Heat 3 of 4",
               await page.$eval("#now-racing-group .heat-lineup h3", h3 => { return $(h3).text(); }));
  
  assert.equal(true, await page.$eval("#is-currently-racing", r => { return $(r).prop('checked'); }));

  var fakeAjax = {
    _expected_config: {},
    _return_value: false,
    _pending: false,  // A Promise that resolves when expectation is met.
    _resolve: false,

    onAjax: function(url, config) {
      if (debugging) {
        console.log("onAjax fires with " + JSON.stringify(config));
      }
      assert.equal(this._expected_config, config);
      this._resolve(true);
      return this._return_value;
    },

    expect: function(ex_config, ret_value) {
      this._expected_config = ex_config;
      this._return_value = ret_value;
      this._pending = new Promise(function(resolve, reject) {
        fakeAjax._resolve = resolve;
      });
    },

    completion: function() {
      return this._pending;
    },
  };

  await page.exposeFunction('onAjax', (url, config) => {
    return fakeAjax.onAjax(url, config);
  });

  // After "Skip Heat" jumps from heat 3 to heat 4:
  fakeAjax.expect(
    {'type': 'POST',
     'data': {'action': 'select-heat',
              'heat': 'next'}},
      '<?xml version="1.0" encoding="UTF-8"?>\n' +
      '<coordinator_poll>\n' +
      '  <current-heat now-racing="0" use-master-sched="0" classid="3" roundid="3" round="1"' +
      '                tbodyid="3" heat="4" number-of-heats="4">Bears and Frèr</current-heat>\n' +
      '  <racer lane="2" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime=""/>\n' +
      '  <racer lane="3" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime=""/>\n' +
      '  <racer lane="4" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime=""/>\n' +
      '  <timer-state lanes="4" last_contact="1526246081" state="3"\n' +
      '         icon="img/status/ok.png">Staging</timer-state>\n' +
      '  <replay-state last_contact="0" state="1" icon="img/status/not_connected.png"\n' +
      '         connected="">NOT CONNECTED</replay-state>\n' +
      '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="18"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="17"\n' +
      '         passed="3" unscheduled="0" heats_scheduled="4" heats_run="2"/>\n' +
      '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="15"\n' +
      '         passed="2" unscheduled="2" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"\n' +
      '         passed="5" unscheduled="5" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="7" classid="7" class="TheLastClass" round="1" roster_size="0"\n' +
      '         passed="0" unscheduled="0" heats_scheduled="0" heats_run="0"/>\n' +
      '</coordinator_poll>');

  var skip_button = await page.$("input[type='button'][value='Skip Heat']");
  skip_button.click();

  await fakeAjax.completion();
  
  await page.waitFor(() => { return !$("#is-currently-racing").prop('checked'); });
  await page.waitFor(() => {
    return $("#now-racing-group .heat-lineup h3").text() == "Heat 4 of 4";
  });

  // After "Previous Heat" button jumps back to heat 3
  fakeAjax.expect(
    {'type': 'POST',
     'data': {'action': 'select-heat',
              'heat': 'prev'}},
    '<?xml version="1.0" encoding="UTF-8"?>\n' +
      '<coordinator_poll>\n' +
      '  <current-heat now-racing="0" use-master-sched="0" classid="3" roundid="3" round="1"' +
      '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
      '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime=""/>\n' +
      '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime=""/>\n' +
      '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime=""/>\n' +
      '  <timer-state lanes="4" last_contact="1526246081" state="3"\n' +
      '         icon="img/status/ok.png">Staging</timer-state>\n' +
      '  <replay-state last_contact="0" state="1" icon="img/status/not_connected.png"\n' +
      '         connected="">NOT CONNECTED</replay-state>\n' +
      '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="18"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="17"\n' +
      '         passed="3" unscheduled="0" heats_scheduled="4" heats_run="2"/>\n' +
      '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="15"\n' +
      '         passed="2" unscheduled="2" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"\n' +
      '         passed="5" unscheduled="5" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="7" classid="7" class="TheLastClass" round="1" roster_size="0"\n' +
      '         passed="0" unscheduled="0" heats_scheduled="0" heats_run="0"/>\n' +
      '</coordinator_poll>');

  var prev_button = await page.$("input[type='button'][value='Previous Heat']");
  prev_button.click();

  await fakeAjax.completion();

  await page.waitFor(() => { return !$("#is-currently-racing").prop('checked'); });
  await page.waitFor(() => {
    return $("#now-racing-group .heat-lineup h3").text() == "Heat 3 of 4";
  });

  // Click "Now Racing" button to resume racing
  fakeAjax.expect(
    {'type': 'POST',
     'data': {'action': 'select-heat',
              'now_racing': 1}},
    '<?xml version="1.0" encoding="UTF-8"?>\n' +
      '<coordinator_poll>\n' +
      '  <current-heat now-racing="1" use-master-sched="0" classid="3" roundid="3" round="1"' +
      '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
      '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime=""/>\n' +
      '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime=""/>\n' +
      '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime=""/>\n' +
      '  <timer-state lanes="4" last_contact="1526246081" state="3"\n' +
      '         icon="img/status/ok.png">Staging</timer-state>\n' +
      '  <replay-state last_contact="0" state="1" icon="img/status/not_connected.png"\n' +
      '         connected="">NOT CONNECTED</replay-state>\n' +
      '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="18"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="17"\n' +
      '         passed="3" unscheduled="0" heats_scheduled="4" heats_run="2"/>\n' +
      '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="15"\n' +
      '         passed="2" unscheduled="2" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"\n' +
      '         passed="5" unscheduled="5" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="7" classid="7" class="TheLastClass" round="1" roster_size="0"\n' +
      '         passed="0" unscheduled="0" heats_scheduled="0" heats_run="0"/>\n' +
      '</coordinator_poll>');

  var racing_button = await page.$("#is-currently-racing");
  racing_button.click();

  await fakeAjax.completion();

  await page.waitFor(() => { return $("#is-currently-racing").prop('checked'); });

  // After manual results:
  fakeAjax.expect(
    {'type': 'POST',
     'data': 'action=result.write&lane1=1.234&lane2=2.34&lane3=4.321'},
    '<?xml version="1.0" encoding="UTF-8"?>\n' +
      '<coordinator_poll>\n' +
      '  <current-heat now-racing="1" use-master-sched="0" classid="3" roundid="3" round="1"' +
      '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
      '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime="1.234"/>\n' +
      '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime="2.34"/>\n' +
      '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime="4.321"/>\n' +
      '  <timer-state lanes="4" last_contact="1526246081" state="3"\n' +
      '         icon="img/status/ok.png">Staging</timer-state>\n' +
      '  <replay-state last_contact="0" state="1" icon="img/status/not_connected.png"\n' +
      '         connected="">NOT CONNECTED</replay-state>\n' +
      '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="18"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="17"\n' +
      '         passed="3" unscheduled="0" heats_scheduled="4" heats_run="2"/>\n' +
      '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="15"\n' +
      '         passed="2" unscheduled="2" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"\n' +
      '         passed="5" unscheduled="5" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="7" classid="7" class="TheLastClass" round="1" roster_size="0"\n' +
      '         passed="0" unscheduled="0" heats_scheduled="0" heats_run="0"/>\n' +
      '</coordinator_poll>');
  
  var manual_results = await page.$("input[type='button'][value='Manual Results']");
  manual_results.click();
  await page.waitFor(() => {
    var manual_results_modal = $("#manual_results_modal");
    return !manual_results_modal.hasClass('hidden') && manual_results_modal.css('opacity') >= 1;
  });

  // Cancel button dismisses #manual_results_modal
  await page.evaluate(() => { $("#manual_results_modal input[type='button'][value='Cancel']").click(); });
  await page.waitFor(() => { return $(".modal_frame").hasClass('hidden'); });


  // Re-open the #manual_results_modal
  manual_results.click();
  await page.waitFor(() => {
    var manual_results_modal = $("#manual_results_modal");
    return !manual_results_modal.hasClass('hidden') && manual_results_modal.css('opacity') >= 1;
  });

  await page.evaluate(() => { $("input[name='lane1']").val('1.234'); });
  await page.evaluate(() => { $("input[name='lane2']").val('2.34'); });
  await page.evaluate(() => { $("input[name='lane3']").val('4.321'); });

  await page.evaluate(() => {
    $("#manual_results_modal input[type='submit']").click();
  });

  await fakeAjax.completion();

  // Check manual results dialog dismissed
  await page.waitFor(() => { return $(".modal_frame").hasClass('hidden'); });

  await page.waitFor(() => {
    var row1 = $("#now-racing-group table tr")[1];
    var cell1_3 = $(row1).find("td")[3];
    var row2 = $("#now-racing-group table tr")[2];
    var cell2_3 = $(row2).find("td")[3];
    return $(cell1_3).text() == "1.234" && $(cell2_3).text() == "2.34";
  });

  // Re-open the #manual_results_modal
  manual_results.click();
  await page.waitFor(() => {
    var manual_results_modal = $("#manual_results_modal");
    return !manual_results_modal.hasClass('hidden') && manual_results_modal.css('opacity') >= 1;
  });

  fakeAjax.expect(
    {'type': 'POST',
     'data': {"action": "result.delete",
              "roundid": "current",
              "heat": "current"}},
    '<?xml version="1.0" encoding="UTF-8"?>\n' +
      '<coordinator_poll>\n' +
      '  <current-heat now-racing="1" use-master-sched="0" classid="3" roundid="3" round="1"' +
      '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
      '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime=""/>\n' +
      '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime=""/>\n' +
      '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime=""/>\n' +
      '  <timer-state lanes="4" last_contact="1526246081" state="3"\n' +
      '         icon="img/status/ok.png">Staging</timer-state>\n' +
      '  <replay-state last_contact="0" state="1" icon="img/status/not_connected.png"\n' +
      '         connected="">NOT CONNECTED</replay-state>\n' +
      '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="18"\n' +
      '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5"/>\n' +
      '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="17"\n' +
      '         passed="3" unscheduled="0" heats_scheduled="4" heats_run="2"/>\n' +
      '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="15"\n' +
      '         passed="2" unscheduled="2" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"\n' +
      '         passed="5" unscheduled="5" heats_scheduled="0" heats_run="0"/>\n' +
      '  <round roundid="7" classid="7" class="TheLastClass" round="1" roster_size="0"\n' +
      '         passed="0" unscheduled="0" heats_scheduled="0" heats_run="0"/>\n' +
      '</coordinator_poll>');
  
  await page.evaluate(() => { $("#discard-results").click(); });
  await page.waitFor(() => { return $(".modal_frame").hasClass('hidden'); });

  await fakeAjax.completion();

  await page.waitFor(() => {
    var row1 = $("#now-racing-group table tr")[1];
    var cell1_3 = $(row1).find("td")[3];
    var row2 = $("#now-racing-group table tr")[2];
    var cell2_3 = $(row2).find("td")[3];
    return $(cell1_3).text() == "" && $(cell2_3).text() == "";
  });

  // TODO input[type='button'][value='Add New Rounds']

  // TODO input[type='button'][value='Replay Settings']
  
  // =============================== Schedule a new round by clicking buttons =====================================
  fakeAjax.expect(
    {'type': 'POST',
     'data': {'action': 'schedule.generate',
              'roundid': 5,
              'nrounds': '1' } },
    false);  // No xml response

  // Click control group to expose schedule button
  await open_scheduling_control_group(".control_group[data-roundid='5']");

  // Click Schedule button to open schedule modal
  await page.evaluate(() => {
    $(".control_group[data-roundid='5'] input[type='button'][value='Schedule']").click();
  });
  
  // Wait for the modal to appear
  await page.waitFor(() => {
    var schedule_modal = $("#schedule_modal");
    return !schedule_modal.hasClass('hidden') && schedule_modal.css('opacity') >= 1;
  });

  // Click the Schedule + Race button
  var dialog_schedule_and_race = 
      await page.waitFor("#schedule_modal input[type='submit'][data-race='true']",
                         {visible: true});
  dialog_schedule_and_race.click();

  await fakeAjax.completion();


  // ================================= Simulated poll =============================================================

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

  await page.evaluate(function(xml) {
    process_coordinator_poll_response((new DOMParser()).parseFromString(xml, 'text/xml')); },
                poll_result);

  assert.equal({'now-racing': [2],
                'ready-to-race': [],
                'not-yet-scheduled': [7, 5, 6, 8],
                'done-racing': [1, 3, 4]},
               await page.evaluate(function() { return g_rounds_layout; }));

  assert.equal(8, await page.evaluate(function() {
    return $(".scheduling_control_group .control_group.scheduling_control").length;
  }));
  assert.equal(8, await page.$$eval(".scheduling_control_group .control_group.scheduling_control",
                                    controls => { return controls.length; }));

  assert.equal([[],  // Headers row
                ["1", "Willard Woolfolk", "282", "3.977"],
                ["2", "Blake Burling", "207", "3.646"],
                ["3", "Elliot Eastman", "227", "2.295"],
                ["4", "Dexter Dawes", "222", "3.720"]],
               await page.evaluate(function() {
                 var table = [];
                 $("#now-racing-group .heat-lineup table tr").each(function(index, tr) {
                   var row = [];
                   $(tr).find("td").each(function(index, td) {
                     row.push($(td).text());
                   });
                   table.push(row);
                 });
                 return table;
               }));

  assert.equal("1", await page.$eval("#now-racing-group .roundno", span => { return $(span).text(); }));
  assert.equal("White's Wolves",
               await page.$eval("#now-racing-group > div > h3", h3 => { return $(h3).text(); }));
  assert.equal("17 racer(s), 13 passed, 13 in schedule.13 heats scheduled, 13 run.",
               await page.$eval("#now-racing-group p", p => { return $(p).text(); }));

  assert.equal([
    // Now racing
    "2", 0,
    // Master schedule
    "-1", 1,"Next Up",
    // Not yet scheduled
    "7", 2,"Schedule","Delete Round",
    "5", 0,  // No passed racers
    "6", 0,
    "8", 2,"Schedule","Delete Round",
    // Done racing
    "1", 1,"Make Changes",
    "3", 1,"Make Changes",
    "4", 1,"Make Changes"],
               await page.evaluate(function() {
                 var buttons = [];
                 $("div.control_group[data-roundid]").each(function() {
                   buttons.push($(this).attr('data-roundid'));
                   buttons.push($(this).find("input[type='button']").length);
                   $(this).find("input[type='button']").each(function() {
                     buttons.push($(this).val());
                   });
                 });
                 return buttons;
               }));

  assert.equal(["Add New Rounds"],
               await page.evaluate(function() {
                 var buttons = [];
                 $("#supplemental-control-group input[type='button']").each(function() {
                   buttons.push($(this).val());
                 });
                 return buttons;
               }));
                 
  assert.includes("east", await page.$eval("#master-schedule-group .scheduling_control img",
                                           img => { return $(img).prop('src'); }));

  await open_scheduling_control_group("#master-schedule-group .scheduling_control");

  assert.includes("south", await page.$eval("#master-schedule-group .scheduling_control img",
                                            img => { return $(img).prop('src'); }));

  // A new poll response shouldn't close the control
  await page.evaluate(function(xml) {
    process_coordinator_poll_response((new DOMParser()).parseFromString(xml, 'text/xml')); },
                poll_result);

  assert.includes("south", await page.$eval("#master-schedule-group .scheduling_control img",
                                            img => { return $(img).prop('src'); }));

  var master_schedule_group = await page.waitFor("#master-schedule-group .scheduling_control");
  master_schedule_group.click();

  await page.waitFor(() => { return $('#master-schedule-group .scheduling_control img').prop('src')
                             .indexOf("east") > 0; });

  assert.includes("east", await page.$eval("#master-schedule-group .scheduling_control img",
                                           img => { return $(img).prop('src'); }));
  
  if (!debugging) {
    await browser.close();
  }
});
