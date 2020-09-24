
const assert = require('./assert.js');

const puppeteer = require('puppeteer');

const fakeAjax = require('./fake-ajax.js');

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

fakeAjax.setDebugging(debugging);

var role = 'RaceCoordinator';
var password = 'doyourbest';

// There's apparently one or more race conditions in the test, but I can't find
// them.  The slowMo makes the test a lot more reliable in the interim.
puppeteer.launch({devtools: debugging, slowMo: 200}).then(async browser => {
  const all_pages = await browser.pages();
  const page = all_pages[0];
  page.setViewport({width: 1200,
                    height: 1800});
  if (debugging) {
    page.on('console', msg => { console.log('REMOTE: ' + msg._text); });
  }

  await page.goto(root + '/login.php');
  await page.evaluate(function (role, password) {
    window.handle_login(role, password);
  }, role, password);
  await page.waitForNavigation();

  await page.evaluateOnNewDocument(() => {
    window.phantom_testing = true;
  });

  await page.goto(root + '/coordinator.php');

  await fakeAjax.installOn(page);

  async function open_scheduling_control_group(selector) {
    await page.evaluate((selector) => { $(selector).click(); }, selector);
    
    // Wait for control group to open
    await page.waitFor((selector) => { console.log(selector + " height = " +  $(selector).height()); return $(selector).height() > 150; }, {}, selector);
  }

  async function modal_open(selector) {
    await page.waitFor((selector) => { return !$(selector).closest(".modal_frame").hasClass('hidden'); },
                       {}, selector);
    await page.waitFor(() => { return $("#modal_background").css('display') == 'block'; });
  }
  async function all_modals_closed() {
    await page.waitFor(() => { return $(".modal_frame").not(".hidden").length == 0; });
    await page.waitFor(() => { return $("#modal_background").css('display') == 'none'; });
  }
  
  // =================================================== First simulated poll =====================================
  await page.evaluate(function(xml) { process_coordinator_poll_response((new DOMParser()).parseFromString(xml, 'text/xml')); },
                '<?xml version="1.0" encoding="UTF-8"?>\n' +
                '<coordinator_poll>\n' +
                '  <current-heat now-racing="1" use-master-sched="0" use-points="0" classid="3" roundid="3" round="1"' +
                '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
                '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime="" finishplace=""/>\n' +
                '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime="" finishplace=""/>\n' +
                '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime="" finishplace=""/>\n' +
                '  <timer-state lanes="4" last_contact="1526246081" state="3"\n' +
                '         icon="img/status/ok.png">Staging</timer-state>\n' +
                '  <replay-state last_contact="0" state="1" icon="img/status/not_connected.png"\n' +
                '         connected="">NOT CONNECTED</replay-state>\n' +
                '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"\n' +
                '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5">' +
                      'Lions &amp; Tigers, Round 1</round>\n' +
                '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="18"\n' +
                '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5">' +
                      'White\'s Wolves, Round 1</round>\n' +
                '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="17"\n' +
                '         passed="3" unscheduled="0" heats_scheduled="4" heats_run="2">' +
                      'Bears and Frèr, Round 1</round>\n' +
                '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="15"\n' +
                '         passed="2" unscheduled="2" heats_scheduled="0" heats_run="0">' +
                      'Webelos (&quot;Webes</round>\n' +
                '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"\n' +
                '         passed="5" unscheduled="5" heats_scheduled="0" heats_run="0">' +
                      'Arrows &lt;&lt;--&lt;&lt;</round>\n' +
                '  <round roundid="7" classid="7" class="TheLastClass" round="1" roster_size="0"\n' +
                '         passed="0" unscheduled="0" heats_scheduled="0" heats_run="0">' +
                      'TheLastClass, Round 1</round>\n' +
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
                ["1","343","Juan Jacobsen",""],
                ["2","139","Jeffress Jamison",""],
                ["3","303","Antoine Akiyama",""],
                ["4","","",""]],
               await page.evaluate(function() {
                 var table = [];
                 $("#now-racing-group .heat-lineup table").first().find("tr").each(function(index, tr) {
                   var row = [];
                   $(tr).find("td").each(function(index, td) {
                     row.push($(td).text());
                   });
                   table.push(row);
                 });
                 return table;
               }));

  assert.equal("Bears and Frèr, Round 1",
               await page.$eval("#now-racing-group > div > h3", h3 => { return $(h3).text(); }));
  assert.equal("4 heats scheduled, 2 run.",
               await page.$eval("#now-racing-group p", p => { return $(p).text(); }));

  assert.equal([
    // Now racing
    "3", 0,
    // Not yet scheduled
    "4", 1, "Schedule",
    "5", 1, "Schedule",
    "7", 0,  // No passed racers
    // Done racing
    "1", 2, "Make Changes", "Repeat Round",
    "2", 2, "Make Changes", "Repeat Round"],
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

  assert.equal(["Add New Rounds", "Repeat Round"],
               await page.evaluate(function() {
                 var buttons = [];
                 $("#supplemental-control-group input[type='button']").each(function() {
                   buttons.push($(this).val());
                 });
                 return buttons;
               }));

  // ===============================  =====================================

  assert.equal("Heat 3 of 4",
               await page.$eval("#now-racing-group .heat_number h3", h3 => { return $(h3).text(); }));
  
  assert.equal(true, await page.$eval("#is-currently-racing", r => { return $(r).prop('checked'); }));

  await fakeAjax.testForAjax(async () => {
    var skip_button = await page.$("#skip_heat_button");
    skip_button.click();
  },
                       {'type': 'POST',
                        'data': {'action': 'heat.select',
                                 'heat': 'next'}},
                       '<?xml version="1.0" encoding="UTF-8"?>\n' +
                       '<coordinator_poll>\n' +
                       '  <current-heat now-racing="0" use-master-sched="0" use-points="0" classid="3" roundid="3" round="1"' +
                       '                tbodyid="3" heat="4" number-of-heats="4">Bears and Frèr</current-heat>\n' +
                       '  <racer lane="2" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime="" finishplace=""/>\n' +
                       '  <racer lane="3" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime="" finishplace=""/>\n' +
                       '  <racer lane="4" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime="" finishplace=""/>\n' +
                       '  <timer-state lanes="4" last_contact="1526246081" state="3"\n' +
                       '         icon="img/status/ok.png">Staging</timer-state>\n' +
                       '  <replay-state last_contact="0" state="1" icon="img/status/not_connected.png"\n' +
                       '         connected="">NOT CONNECTED</replay-state>\n' +
                       '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"\n' +
                       '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5">' +
                             'Lions &amp; Tigers, Round 1</round>\n' +
                       '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="18"\n' +
                       '         passed="5" unscheduled="0" heats_scheduled="5" heats_run="5">' +
                             'White\'s Wolves, Round 1</round>\n' +
                       '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="17"\n' +
                       '         passed="3" unscheduled="0" heats_scheduled="4" heats_run="2">' +
                             'Bears and Frèr, Round 1</round>\n' +
                       '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="15"\n' +
                       '         passed="2" unscheduled="2" heats_scheduled="0" heats_run="0">' +
                             'Webelos (&quot;Webes, Round 1</round>\n' +
                       '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"\n' +
                       '         passed="5" unscheduled="5" heats_scheduled="0" heats_run="0">' +
                             'Arrows &lt;&lt;--&lt;&lt;, Round 1</round>\n' +
                       '  <round roundid="7" classid="7" class="TheLastClass" round="1" roster_size="0"\n' +
                       '         passed="0" unscheduled="0" heats_scheduled="0" heats_run="0">' +
                             'TheLastClass, Round 1</round>\n' +
                       '</coordinator_poll>');
  
  await page.waitFor(() => { return !$("#is-currently-racing").prop('checked'); });
  await page.waitFor(() => {
    return $("#now-racing-group .heat-text h3").text() == "Heat 4 of 4";
  });

  // After "Previous Heat" button jumps back to heat 3
  await fakeAjax.testForAjax(async () => {
    var prev_button = await page.$("#prev_heat_button");
    prev_button.click();
  },
                       {'type': 'POST',
                        'data': {'action': 'heat.select',
                                 'heat': 'prev'}},
                       '<?xml version="1.0" encoding="UTF-8"?>\n' +
                       '<coordinator_poll>\n' +
                       '  <current-heat now-racing="0" use-master-sched="0" use-points="0" classid="3" roundid="3" round="1"' +
                       '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
                       '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime="" finishplace=""/>\n' +
                       '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime="" finishplace=""/>\n' +
                       '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime="" finishplace=""/>\n' +
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

  await page.waitFor(() => { return !$("#is-currently-racing").prop('checked'); });
  await page.waitFor(() => {
    return $("#now-racing-group .heat-text h3").text() == "Heat 3 of 4";
  });

  if (false) {
  // Click "Now Racing" button to resume racing
  await fakeAjax.testForAjax(async () => {
    var racing_button = await page.$(".ui-flipswitch > #is-currently-racing");
    racing_button.click();
  },
                       {'type': 'POST',
                        'data': {'action': 'heat.select',
                                 'now_racing': 1}},
                       '<?xml version="1.0" encoding="UTF-8"?>\n' +
                       '<coordinator_poll>\n' +
                       '  <current-heat now-racing="1" use-master-sched="0" use-points="0" classid="3" roundid="3" round="1"' +
                       '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
                       '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime="" finishplace=""/>\n' +
                       '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime="" finishplace=""/>\n' +
                       '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime="" finishplace=""/>\n' +
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

  await page.waitFor(() => { return $("#is-currently-racing").prop('checked'); });
  }

  // After manual results:
  var manual_results = await page.$("input[type='button'][value='Manual Results']");
  await fakeAjax.testForAjax(async () => {
    manual_results.click();
    await page.waitFor(() => {
      var manual_results_modal = $("#manual_results_modal");
      return !manual_results_modal.hasClass('hidden') && manual_results_modal.css('opacity') >= 1;
    });

    // Cancel button dismisses #manual_results_modal
    await page.evaluate(() => { $("#manual_results_modal input[type='button'][value='Cancel']").click(); });
    await all_modals_closed();


    // Re-open the #manual_results_modal
    manual_results.click();
    await page.waitFor(() => {
      var manual_results_modal = $("#manual_results_modal");
      return !manual_results_modal.hasClass('hidden') && manual_results_modal.css('opacity') >= 1;
    });

    await page.evaluate(() => {
      $("input[name='lane1']").val('1.234');
      $("input[name='lane2']").val('2.34');
      $("input[name='lane3']").val('4.321');
      $("#manual_results_modal input[type='submit']").click();
      return 0;
    });
  },
                       {'type': 'POST',
                        'data': 'action=result.write&lane1=1.234&lane2=2.34&lane3=4.321'},
                       '<?xml version="1.0" encoding="UTF-8"?>\n' +
                       '<coordinator_poll>\n' +
                       '  <current-heat now-racing="1" use-master-sched="0" use-points="0" classid="3" roundid="3" round="1"' +
                       '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
                       '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime="1.234" finishplace="1"/>\n' +
                       '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime="2.34" finishplace="2"/>\n' +
                       '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime="4.321" finishplace="3"/>\n' +
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

  // Check manual results dialog dismissed
  await all_modals_closed();

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

  await fakeAjax.testForAjax(async () => {
    await page.evaluate(() => { $("#discard-results").click(); });
    await all_modals_closed();
  },
                       {'type': 'POST',
                        'data': {"action": "result.delete",
                                 "roundid": "current",
                                 "heat": "current"}},
                       '<?xml version="1.0" encoding="UTF-8"?>\n' +
                       '<coordinator_poll>\n' +
                       '  <current-heat now-racing="1" use-master-sched="0" use-points="0" classid="3" roundid="3" round="1"' +
                       '                tbodyid="3" heat="3" number-of-heats="4">Bears and Frèr</current-heat>\n' +
                       '  <racer lane="1" name="Juan Jacobsen" carname="" carnumber="343" photo="" finishtime="" finishplace=""/>\n' +
                       '  <racer lane="2" name="Jeffress Jamison" carname="" carnumber="139" photo="" finishtime="" finishplace=""/>\n' +
                       '  <racer lane="3" name="Antoine Akiyama" carname="" carnumber="303" photo="" finishtime="" finishplace=""/>\n' +
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

  await page.waitFor(() => {
    var row1 = $("#now-racing-group table tr")[1];
    var cell1_3 = $(row1).find("td")[3];
    var row2 = $("#now-racing-group table tr")[2];
    var cell2_3 = $(row2).find("td")[3];
    return $(cell1_3).text() == "" && $(cell2_3).text() == "";
  });

  // Click "Add New Rounds" button, see the dialog, dismiss it.
  await page.evaluate(() => { $("input[type='button'][value='Add New Rounds']").click(); });
  await modal_open("#choose_new_round_modal");
  await page.evaluate(() => { $("#choose_new_round_modal input[type='button'][value='Cancel'").click(); });
  await all_modals_closed();

  // Click "Add New Rounds" button, see the dialog, choose a den, see #new_round_modal, dismiss it.
  await page.evaluate(() => { $("input[type='button'][value='Add New Rounds']").click(); });
  await modal_open("#choose_new_round_modal");
  await page.evaluate(() => { $($("#choose_new_round_modal input[type='button']")[1]).click(); });
  await modal_open("#new_round_modal");
  await page.evaluate(() => { $("#new_round_modal input[type='button'][value='Cancel']").click(); });
  await all_modals_closed();

  await fakeAjax.testForAjax(async () => {
    await page.evaluate(() => { $("input[type='button'][value='Add New Rounds']").click(); });
    await modal_open("#choose_new_round_modal");

    // Buttons in the "Add New Rounds" modal:
    //  Lions & Tigers
    //  White's Wolves [1]
    //  Aggregate Round
    //  Cancel
    await page.evaluate(() => { $($("#choose_new_round_modal input[type='button']")[1]).click(); });
    await modal_open("#new_round_modal");
    await page.evaluate(() => { $("#new_round_modal input[type='number'][name='top']").val('4'); });
    await page.waitFor(() => { return $(".multi_den_only").hasClass('hidden') && !$(".single_den_only").hasClass('hidden'); });
    await page.evaluate(() => { $("#new_round_modal input[type='submit']").click(); });
  },
                             {'type': 'POST',
                              'data': 'action=roster.new&roundid=2&roundid_1=on&roundid_2=on&top=4&classname=Grand+Finals'},
                             '<?xml version="1.0" encoding="UTF-8"?>\n' +
                             '<action-response action="roster.new" roundid="2" roundid_1="on" roundid_2="on" top="4" classname="Grand Finals">\n' +
                             '  <finalist racerid="7" bucket_number="1"/>\n' +
                             '  <finalist racerid="17" bucket_number="1"/>\n' +
                             '  <finalist racerid="27" bucket_number="1"/>\n' +
                             '  <finalist racerid="37" bucket_number="1"/>\n' +
                             '  <non-finalist racerid="47" bucket_number="1"/>\n' +
                             '  <new-round roundid="8"/>\n' +
                             '  <success/>\n' +
                             '  <coordinator_poll>\n' +
                             '    <current-heat now-racing="0" use-master-sched="0" use-points="0" classid="2" roundid="2"' +
                             '                  round="1" tbodyid="2" heat="5" number-of-heats="5">White\'s Wolves</current-heat>\n' +
                             '    <racer lane="1" name="Kelvin Knapp" carname="" carnumber="247" photo="" finishtime="" finishplace=""/>\n' +
                             '    <racer lane="3" name="Darrell &amp; Darrell Delaughter" carname="" carnumber="217" photo="" finishtime="" finishplace=""/>\n' +
                             '    <racer lane="5" name="Ian Ives" carname="" carnumber="237" photo="" finishtime="" finishplace=""/>\n' +
                             '    <timer-state lanes="6" last_contact="0" state="1" icon="img/status/not_connected.png">NOT CONNECTED</timer-state>\n' +
                             '    <replay-state last_contact="0" state="1" icon="img/status/not_connected.png" connected="">NOT CONNECTED</replay-state>\n' +
                             '    <round roundid="8" classid="2" class="White\'s Wolves" round="2" roster_size="4"' +
                             '           passed="4" unscheduled="4" heats_scheduled="0" heats_run="0"/>\n' +
                             '    <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"' +
                             '           passed="5" unscheduled="0" heats_scheduled="5" heats_run="0"/>\n' +
                             '    <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="18"' +
                             '           passed="5" unscheduled="0" heats_scheduled="5" heats_run="0"/>\n' +
                             '    <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="17"' +
                             '           passed="6" unscheduled="6" heats_scheduled="0" heats_run="0"/>\n' +
                             '    <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="15"' +
                             '           passed="2" unscheduled="2" heats_scheduled="0" heats_run="0"/>\n' +
                             '    <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"' +
                             '           passed="5" unscheduled="5" heats_scheduled="0" heats_run="0"/>\n' +
                             '    <round roundid="7" classid="7" class="TheLastClass" round="1" roster_size="0"' +
                             '           passed="0" unscheduled="0" heats_scheduled="0" heats_run="0"/>\n' +
                             '  </coordinator_poll>\n' +
                             '</action-response>');

  // TODO Try creating a new aggregate round

  // TODO input[type='button'][value='Replay Settings']
  
  // =============================== Schedule a new round by clicking buttons =====================================
  await fakeAjax.testForAjax(async () => {
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
    await dialog_schedule_and_race.click();
  },
                       {'type': 'POST',
                        'data': {'action': 'schedule.generate',
                                 'roundid': 5,
                                 'n_times_per_lane': '1' } },
                         false);  // No xml response

  // ================================= Simulated poll =============================================================

    var poll_result = '<?xml version="1.0" encoding="UTF-8"?>\n' +
      '<coordinator_poll>\n' +
      '  <current-heat now-racing="1" use-master-sched="1" use-points="0" classid="2" roundid="2" round="1"' +
      '     tbodyid="1" heat="13" number-of-heats="13">White\'s Wolves</current-heat>\n' +
      '  <racer lane="1" name="Willard Woolfolk" carname="" carnumber="282"' +
      '     photo="" finishtime="3.977" finishplace="4"/>\n' +
      '  <racer lane="2" name="Blake Burling" carname="" carnumber="207"' +
      '     photo="" finishtime="3.646" finishplace="2"/>\n' +
      '  <racer lane="3" name="Elliot Eastman" carname="" carnumber="227"' +
      '     photo="" finishtime="2.295" finishplace="1"/>\n' +
      '  <racer lane="4" name="Dexter Dawes" carname="" carnumber="222"' +
      '     photo="" finishtime="3.720" finishplace="3"/>\n' +
      '  <timer-state lanes="4" last_contact="1496356697" state="1"' +
      '     icon="img/status/not_connected.png">NOT CONNECTED</timer-state>\n' +
      '  <replay-state last_contact="1496360776" state="1"' +
      '     icon="img/status/not_connected.png" connected="">NOT CONNECTED</replay-state>\n' +
      '  <round roundid="7" classid="2" class="White\'s Wolves" round="2" roster_size="3"' +
      '     passed="3" unscheduled="3" heats_scheduled="0" heats_run="0">' +
        'White\'s Wolves</round>\n' +
      '  <round roundid="1" classid="1" class="Lions &amp; Tigers" round="1" roster_size="17"' +
      '     passed="17" unscheduled="0" heats_scheduled="17" heats_run="17">' +
        'Lions &amp; Tigers, Round 1</round>\n' +
      '  <round roundid="2" classid="2" class="White\'s Wolves" round="1" roster_size="17"' +
      '     passed="13" unscheduled="0" heats_scheduled="13" heats_run="13">' +
        'White\'s Wolves, Round 1</round>\n' +
      '  <round roundid="3" classid="3" class="Bears and Frèr" round="1" roster_size="16"' +
      '     passed="2" unscheduled="0" heats_scheduled="4" heats_run="4">' +
        'Bears and Frèr</round>\n' +
      '  <round roundid="4" classid="4" class="Webelos (&quot;Webes" round="1" roster_size="16"' +
      '     passed="3" unscheduled="0" heats_scheduled="4" heats_run="4">' +
        'Webelos (&quot;Webes, Round 1</round>\n' +
      '  <round roundid="5" classid="5" class="Arrows &lt;&lt;--&lt;&lt;" round="1" roster_size="16"' +
      '     passed="0" unscheduled="0" heats_scheduled="0" heats_run="0">' +
        'Arrows &lt;&lt;--&lt;&lt;, Round 1</round>\n' +
      '  <round roundid="6" classid="6" class="TheLastClass" round="1" roster_size="0"' +
      '     passed="0" unscheduled="0" heats_scheduled="0" heats_run="0">' +
        'TheLastClass, Round 1</round>\n' +
      '  <round roundid="8" classid="7" class="Grand Finals" aggregate="1" round="1" roster_size="5"' +
      '     passed="5" unscheduled="5" heats_scheduled="0" heats_run="0">' +
        'Grand Finals, Round 1</round>\n' +
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
                ["1", "282", "Willard Woolfolk", "3.977"],
                ["2", "207", "Blake Burling", "3.646"],
                ["3", "227", "Elliot Eastman", "2.295"],
                ["4", "222", "Dexter Dawes", "3.720"]],
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

  assert.equal("White's Wolves, Round 1",
               await page.$eval("#now-racing-group > div > h3", h3 => { return $(h3).text(); }));
  assert.equal("13 heats scheduled, 13 run.",
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
    "1", 2,"Make Changes","Repeat Round",
    "3", 2,"Make Changes","Repeat Round",
    "4", 2,"Make Changes","Repeat Round"],
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

  assert.equal(["Add New Rounds","Repeat Round"],
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
