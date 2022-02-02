// rows is a jquery of the rows for the round and/or rank
// we're trying to display.  Some of the selected rows may be hidden.  goal says
// how many rows we want not to be hidden, or false if we're trying to display
// all.  Returns true if it actually reveals a row.
function maybe_reveal_one_row(rows, goal) {
  console.log("maybe_reveal_one_row, " + rows.length + " row(s), " + rows.not(".hidden").length + " exposed, " + goal + " goal.");
  if (goal === false || rows.not(".hidden").length < goal) {
    var next = rows.filter(".hidden:last");
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
  if (interval) {
    clearInterval(interval);
    // Quickly finish scrolling until normal row order is restored
    while ($(".main_table tr[data-roundid]:first").is(":not([data-first])")) {
      $(".main_table tr[data-roundid]:last").detach().insertAfter(".main_table tr.headers");
    }
  }
}

function need_scrolling(rows) {
  var offscreen = false;
  rows.each(function(index, element) {
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
    exposed: 0,
    catalog_entry: {},

    // If scrolling is taking place, this holds the interval object
    scrolling_interval: false,
    reveal_timeout: false,

    autoreveal: function() {
      var rows = select_standings_by_catalog_entry(poller.catalog_entry);
      if (maybe_reveal_one_row(rows, poller.exposed)) {
        poller.reveal_timeout = setTimeout(function() { poller.autoreveal() }, 1500);
      } else {
        // If there are no more hidden rows for selection, then maybe start
        // scrolling if the exposed list is long enough.
        if (rows.filter(".hidden").length == 0) {
          rows.first().addClass("first_visible");
          if (!poller.scrolling_interval && need_scrolling(rows)) {
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
              success: function(data) {
                var changed = false;
                if (data.hasOwnProperty('catalog-entry')) {
                  var entry = data['catalog-entry'];
                  if (entry.key != poller.catalog_entry.key) {
                    poller.catalog_entry = entry;
                    changed = true;
                    stop_scrolling(poller.scrolling_interval);
                    poller.scrolling_interval = false;
                    $("tr").not(".headers").addClass('hidden');
                  }
                }

                if (data.hasOwnProperty('exposed')) {
                  var new_exposed = data.exposed === '' ? false : data.exposed;
                  if (new_exposed !== poller.exposed) {
                    changed = true;
                    poller.exposed = new_exposed;
                  }
                }

                if (changed) {
                  $(".main_table tr").removeClass("first_visible");
                  $(".main_table").css({'margin-left': 'auto',
                                        'margin-right': 'auto'});
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
