

$(function() {
  setInterval(function() {
    $.ajax("action.php",
           {type: 'GET',
            data: {query: 'poll.results'},
            success: function(data) {
              process_newresults(data);
            }
           });
  }, 1000);
});

function process_newresults(data) {
  var rounds = data.getElementsByTagName("round");

  $("tbody[data-roundid]").each(function() {
    var roundid = $(this).attr('data-roundid');
    for (var i = 0; i < rounds.length; ++i) {
      if (rounds[i].getAttribute('roundid') == roundid) {
        return;
      }
    }
    console.log("tbody for non-existent roundid " + roundid);
    location.reload(true);
  });

  rewrite_table_sections(rounds, 0, function() {
    update_results(data.getElementsByTagName("result"));

    var current_xml = data.getElementsByTagName("current-heat")[0];
    var current = {tbodyid: current_xml.getAttribute("tbodyid"),
                   roundid: current_xml.getAttribute("roundid"),
                   round: current_xml.getAttribute("round"),
                   heat: current_xml.getAttribute("heat"),
                   classname: current_xml.textContent};

    var next_heat_xml = data.getElementsByTagName("next-heat")[0];

    notice_change_current_tbody(current.tbodyid, current.round,
                                current.classname);
  });
}

// Each tbody in racer-results has an id with the roundid.
function rewrite_table_sections(rounds, index, completed) {
  if (index < rounds.length) {
    var round = rounds.item(index);
    var roundid = round.getAttribute('roundid');
    var tbody = $("#tbody_" + roundid);

    if (round.textContent && tbody.attr('data-signature') != round.textContent) {
      if (tbody.length == 0) {
        console.log("Reloading page for new roundid " + round.textContent);
        // Wholly-new roundid implies added a new round; just reload rather than
        // try to guess where the new tbody should go.
        location.reload(true);
        return;
      }
      console.log("tbody load for " + roundid
                  + " because old " + tbody.attr('data-signature') + ' != new ' + round.textContent);

      tbody.attr('data-signature', round.textContent);
      tbody.load(location.href + " #tbody_" + roundid + " tr", /* data */ '',
                 function() { rewrite_table_sections(rounds, 1 + index, completed); });

      rewrite_table_sections(rounds, 1 + index, completed);
    } else {
      rewrite_table_sections(rounds, 1 + index, completed);
    }
  } else {
    completed();
  }
}

function update_results(result_elements) {
  for (var i = 0; i < result_elements.length; ++i) {
    var result = result_elements.item(i);
    $("td.resultid_" + result.getAttribute('resultid') + " span.time")
      .text(result.getAttribute('outcome'));
  }

  if (g_as_kiosk) {
    // Any tbody's with no results should be collapsed or hidden
    $("tbody[data-roundid]").not(".being-scrolled").each(function() {
      var tbody = $(this);
      tbody.toggleClass('hidden',
                        tbody.find("span.time").filter(function() {
                          return $(this).text() != '--';
                        }).length == 0);
    });
  }
}
