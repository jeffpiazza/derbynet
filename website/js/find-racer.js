////////////////////////////////////////////////////
// Code here expects these methods:
//
// search_for_racers(search_string) -- returns number of matches
// maybe_barcode(string) -- case sensitive, returns true if handled as a barcode
// remove_search_highlighting()
// scroll_to_nth_found_racer(index) -- 1-based index
////////////////////////////////////////////////////

function global_keypress(event) {
  if (document.activeElement == document.body ||
      document.activeElement == null) {
    // If no other element holds focus, show and focus on the search box.  This
    // is especially important with barcode scanners, as a lack of focus will
    // ignore the scanned text.
    //
    // We're invoked on a keypress event; the actual input key will be read as
    // part of handling for the keyup that follows.
    $("#find-racer").removeClass('hidden');
    $("#find-racer-text").focus();
  }
}

function intercept_arrow_key(event) {
  switch (event.which) {
  case 38:  // up
    next_or_previous_found_racer(-1);
    event.preventDefault();
    break;
  case 40: // down
    next_or_previous_found_racer(+1);
    event.preventDefault();
    break;
  case 9:  // tab or shift-tab
    next_or_previous_found_racer(event.shiftKey ? -1 : +1);
    event.preventDefault();
    break;
  case 27:  // esc
    cancel_find_racer();
    event.preventDefault();
    break;
  }
}

function cancel_find_racer() {
  $("#find-racer-text").val("");
  $("#find-racer").removeClass("notfound");
  $("#find-racer-index").data("index", 1).text(1);
  $("#find-racer-count").text(0);
  $("#find-racer-message").css({visibility: 'hidden'});
  remove_search_highlighting();
  // We show the find-racer box without sliding (for speed), so let's not bother
  // sliding to hide.
  // $("#find-racer").slideUp(function() { $("#find-racer").addClass('hidden'); });
  $("#find-racer").addClass('hidden');
}

// inc = 1 for next found racer, -1 for previous
function next_or_previous_found_racer(inc) {
  var count = $("#find-racer-index").data("index");
  if (inc > 0 && count < $("span.found-racer").length) {
    ++count;
  } else if (inc < 0 && count > 1) {
    --count;
  } else {
    return;
  }

  $("#find-racer-index").data("index", count).text(count);
  scroll_to_nth_found_racer(count);
}

// In response to each onchange event for the #find-racer-text control, hide the
// table rows that don't contain the value string.
function on_changed_search() {
  remove_search_highlighting();
  var raw_search = $("#find-racer-text").val();

  if (raw_search.length == 0) {
    cancel_find_racer();
  } else if (maybe_barcode(raw_search)) {
    // Barcode may be case-sensitive
    cancel_find_racer();
    return;
  } else {
    var find_count = search_for_racers(raw_search.toLowerCase());
    if (find_count != 0) {
      $("#find-racer").removeClass("notfound");
      $("#find-racer-index").data("index", 1).text(1);
      $("#find-racer-count").text(find_count);
      $("#find-racer-message").css({visibility: 'visible'});
    } else {
      console.log("No match!");
      $("#find-racer").addClass("notfound");
      $("#find-racer-index").data("index", 1).text(1);
      $("#find-racer-count").text(0);
      $("#find-racer-message").css({visibility: 'hidden'});
    }
  }
}

$(function() {
  $(document).on("keypress", global_keypress);

  $("#find-racer-text").on("input", on_changed_search)
    .on("keydown", intercept_arrow_key);

  // jquery mobile would add a distracting "blue glow" around the input form
  // after the text input receives focus.  Ugh.
  $("#find-racer-text").off('focus');
});
