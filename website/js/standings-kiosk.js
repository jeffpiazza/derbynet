// row_selector is a string that selects the rows for the round and/or rank
// we're trying to display.  Some of the selected rows may be hidden.  goal says
// how many rows we want not to be hidden, or false if we're trying to display
// all.  Returns true if it actually reveals a row.
function maybe_reveal_one_row(row_selector, goal) {
  if (goal === false || $(row_selector).not(".hidden").length < goal) {
    var next = $(row_selector + ".hidden:last");
    if (next.length > 0) {
      reveal_one_row(next.removeClass('hidden'));
      return true;
    }
  }
  return false;
}

function reveal_one_row(next) {
  var td = next.find('td');
  var padding = td.css('padding');
  td.css({padding: '0px'})
    .animate({padding: '13px'}, {duration: 400})
    .wrapInner('<div style="display: none;" />')
    .parent()
    .find('td > div')
    .slideDown(400, function() {
      // This callback gets called for each td/div in the row, not just once per row.
      // Remove the div we introduced above, exposing the td contents
      $(this).replaceWith($(this).contents());
    });
}

// Animate the results-by-racer page so rows scroll up in a continuous loop.
function scroll_one_visible_row_of_standings() {
  do {
    var last_row = $(".main_table tr[data-roundid]:last").detach();
    last_row.insertAfter(".main_table tr.headers");
    if (last_row.not(".hidden")) {
      reveal_one_row(last_row);
    }
  } while ($(".main_table tbody:first tr[data-roundid]:last").is(".hidden"));
}

function stop_scrolling(interval) {
  console.log("Stop scrolling");
  if (interval) {
    clearInterval(interval);
    // Quickly finish scrolling until normal row order is restored
    while ($(".main_table tr[data-roundid]:first").is(":not([data-first])")) {
      $(".main_table tr[data-roundid]:last").detach().insertAfter(".main_table tr.headers");
    }
  }
}

function need_scrolling(selector) {
  var offscreen = false;
  $(selector).each(function(index, element) {
    if (element.getBoundingClientRect().top > $(window).height()) {
      offscreen = true;
    }
  });
  return offscreen;
}

function empty_is_false(v) {
  return v == "" ? false : v;
}

$(function() {

  $("tr").not(".headers").addClass('hidden');
  
  var poller = {
    roundid: -1,
    rankid: -1,
    exposed: 0,
    group_name: "",

    // If scrolling is taking place, this holds the interval object
    scrolling_interval: false,
    reveal_timeout: false,

    autoreveal: function() {
      var selector = select_standings(poller.roundid, poller.rankid, poller.group_name);
      if (maybe_reveal_one_row(selector, poller.exposed)) {
        poller.reveal_timeout = setTimeout(function() { poller.autoreveal() }, 1500);
      } else {
        // If there are no more hidden rows for selection, then maybe start
        // scrolling if the exposed list is long enough.
        if ($(selector).filter(".hidden").length == 0) {
          $(selector).first().addClass("first_visible");
          if (!poller.scrolling_interval && need_scrolling(selector)) {
            poller.scrolling_interval = setInterval(scroll_one_visible_row_of_standings, 2000);
          }
        }
      }
    },

    poll: function() {
      $.ajax({type: 'GET',
              url: 'action.php',
              data: {
                query: 'standings.reveal'
              },
              dataType: "xml",
              success: function(data) {
                var changed = false;

                var round_element = data.getElementsByTagName('round');
                if (round_element.length > 0) {
                  var roundid = round_element[0].getAttribute('roundid');
                  var rankid = round_element[0].getAttribute('rankid');
                  if (roundid != poller.roundid) {
                    changed = true;
                    poller.roundid = empty_is_false(roundid);
                    poller.group_name = round_element[0].textContent;
                  }

                  if (rankid != poller.rankid) {
                    changed = true;
                    poller.rankid = empty_is_false(rankid);
                    poller.group_name = round_element[0].textContent;
                  }

                  if (changed) {
                    stop_scrolling(poller.scrolling_interval);
                    poller.scrolling_interval = false;
                    $("tr").not(".headers").addClass('hidden');
                  }
                }

                var exposed_element = data.getElementsByTagName('exposed');
                if (exposed_element.length > 0) {
                  var new_exposed = 0;
                  if (exposed_element[0].getAttribute('count') == "") {
                    new_exposed = false;
                  } else {
                    new_exposed = Number(exposed_element[0].getAttribute('count'));
                  }

                  if (new_exposed !== poller.exposed) {
                    changed = true;
                    poller.exposed = new_exposed;
                  }
                }

                if (changed) {
                  $(".main_table tr").removeClass("first_visible");
                  if (poller.reveal_timeout) {
                    clearTimeout(poller.reveal_timeout);
                    poller.reveal_timeout = false;
                  }
                  poller.autoreveal();
                }
              }
             });
    }};
  setInterval(function() { poller.poll(); }, 300);
});
