// For debugging:
function show_all_rows() {
  $("tr").removeClass('hidden');
  $("td > div.inround").removeClass('hidden');
  $("td > div.insuper").removeClass('hidden').css('color', 'red');
  $("td > div.nonracing-class").removeClass('hidden').css('color', 'green');
  $("select > option[data-aggregate]").each(function (index, elt) {
    var cl = $(elt).attr('data-aggregate');
    $("td > div.nonracing-class[data-agg-classid=" + cl + "]").prepend($(elt).text() + ": ");
  });
}


// selection is jquery of an <option> element from the view selector.
//
// Each option has attributes for data-presentation and value.
function standings_select_on_change(selection, supergroup_label) {
  var rows = select_standings(selection.attr('value'),
                              selection.attr('data-presentation'));
  $("tr").not(".headers").addClass('hidden');
  rows.removeClass('hidden');
}

// key identifies which rows to show
// presentation identifies which columns (and which divs within cells)
function select_standings(key, presentation) {
  console.log('Key = ' + key);
  console.log('Presentation = ' + presentation);

  $("td.col0 div, .col-group,.col-ingroup, .col-subgroup,.col-insubgroup, .col-insuper, .col-inround").addClass('hidden');

  var col_selectors = '';

  if (presentation == 'ff') {
    col_selectors = 'div.insuper, .col-group,.col-ingroup, .col-subgroup,.col-insubgroup';
    // TODO Aggregate racing group likely needs its own presentation
    // TODO Aggregate racing group should show organic group, but organic rounds shouldn't.
    // TODO For aggregate racing round, populate ingroup column with ... ?
  } else if (presentation == 'class') {
    col_selectors = 'div.inround, .col-group,.col-subgroup,.col-insubgroup';
    // TODO For subgroup, maybe just adjust the column header name for col0 to be
    // "in <group-label>", and suppress .col-ingroup.
  } else if (presentation == 'subgroup') {
    col_selectors = 'div.inround, .col-ingroup, .col-insubgroup, .col-insuper';
  } else if (presentation == 'round') {
    col_selectors = 'div.inround, .col-subgroup,.col-insubgroup';
    // TODO For round-subgroup, maybe show .col-inround and suppress col0 altogether?
  } else if (presentation == 'round-subgroup') {
    col_selectors = 'div.inround, .col-insubgroup';
  } else if (presentation == 'qual') {
    col_selectors = '.col-group,.col-ingroup, .col-subgroup,.col-insubgroup';
  }

  $(col_selectors).removeClass('hidden');
  $("div." + key).removeClass('hidden');

  return $("tr." + key);
}

// entry has name, key, presentation
function select_standings_by_catalog_entry(entry) {
  $("div.banner_title").text(entry.name + " Standings");
  return select_standings(entry.key, entry.presentation);
}

