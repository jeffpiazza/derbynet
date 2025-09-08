// rows is a jquery of the rows for the round and/or rank
// we're trying to display.  Some of the selected rows may be hidden.  goal says
// how many rows we want not to be hidden, or false if we're trying to display
// all.  Returns true if it actually reveals a row.
function maybe_reveal_one_row(rows, goal) {
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

function find_catalog_entry(key) {
  for (entry of g_standings_catalog) {
    if (entry.key == key) {
      return entry;
    }
  }
  return false;
}

var g_standings_animator = {
  exposed: false,  // Reveal everything
  catalog_entry: null,

  // If scrolling is taking place, this holds the interval object
  scrolling_interval: false,
  // If not all the rows have been revealed yet, this holds the timeout object
  // for the next row reveal.
  reveal_timeout: false,

  autoreveal: function() {
    var rows = select_standings_by_catalog_entry(g_standings_animator.catalog_entry);
    if (maybe_reveal_one_row(rows, g_standings_animator.exposed)) {
      g_standings_animator.reveal_timeout = setTimeout(
        function() { g_standings_animator.autoreveal() }, 1500);
    } else {
      // If there are no more hidden rows for selection, then maybe start
      // scrolling if the exposed list is long enough.
      if (rows.filter(".hidden").length == 0) {
        rows.first().addClass("first_visible");
        if (!g_standings_animator.scrolling_interval && need_scrolling(rows)) {
          g_standings_animator.scrolling_interval = setInterval(
            scroll_one_visible_row_of_standings, 2000);
        }
      }
    }
  }
};

function on_new_catalog_choice(entry) {
  g_standings_animator.catalog_entry = entry;
  changed = true;
  stop_scrolling(g_standings_animator.scrolling_interval);
  g_standings_animator.scrolling_interval = false;
  $("tr").not(".headers").addClass('hidden');
  
  $(".main_table tr").removeClass("first_visible");
  $(".main_table").css({'margin-left': 'auto',
                        'margin-right': 'auto'});
  if (g_standings_animator.reveal_timeout) {
    clearTimeout(g_standings_animator.reveal_timeout);
    g_standings_animator.reveal_timeout = false;
  }
  g_standings_animator.autoreveal();
}  

$(function() {
  
  $("tr").not(".headers").addClass('hidden');

  KioskPoller.param_callback = function(parameters) {
    var entry;    // {name:, key:, presentation: }
    if (parameters.hasOwnProperty('key')) {
      entry = find_catalog_entry(parameters.key);
    } else if (parameters.hasOwnProperty('classid')) {
      entry = find_catalog_entry(g_class_keys[parameters.classid]);
    } else if (parameters.hasOwnProperty('rankid')) {
      entry = find_catalog_entry(g_rank_keys[parameters.rankid]);
    } else if (parameters.hasOwnProperty('catalog-entry')) {
      entry = parameters['catalog-entry'];
    }

    if (entry && g_standings_animator.catalog_entry &&
        entry.key != g_standings_animator.catalog_entry.key) {
      on_new_catalog_choice(entry);
    }
  };

  // If there's no choice of what standings to display after a few seconds, then
  // pick a default.
  setTimeout(function() {
    if (!g_standings_animator.catalog_entry && g_standings_catalog.length > 0) {
      console.log("After timeout, picking the first standings catalog entry.");
      on_new_catalog_choice(g_standings_catalog[0]);
    }
  }, 5000);
});
