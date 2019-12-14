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


// entry has kind, key, and name, and, depending on kind, maybe roundid, rankid,
// and/or classid.
function select_standings_by_catalog_entry(entry) {
  $(".group_column,.ingroup_column,.subgroup_column,.insubgroup_column").removeClass('hidden');
  $(".inround_column").addClass('hidden');
  
  document.title =  entry.name + ' Standings';
  $("div.banner_title").text(entry.name + ' Standings');

  if (entry.kind == 'supergroup') {
    $("td > div.inround, td > div.nonracing-class").addClass("hidden");
    $("td > div.insuper").removeClass("hidden");
    $(".insuper_column").addClass("hidden");
    return $("tr[data-for-supergroup='1']");
  } else if (entry.kind == 'class' || entry.kind == 'round') {
    $("td > div.inround").removeClass("hidden");
    $("td > div.insuper, td > div.nonracing-class").addClass("hidden");
    if (entry.kind == 'class') {
      $(".insuper_column").removeClass('hidden');
    } else {  // 'round'
      // It could be meaningful to show rank-round standings as a secondary
      // column for 'round', but we don't.
      $(".ingroup_column, .insubgroup_column").addClass('hidden');
    }
    return $("tr[data-roundid='" + entry.roundid + "']");
  } else if (entry.kind == 'rank') {
    // TODO Column 1 should give place in rank here, and place in round/place in
    // class should be moved to another column.  Also, insuper_column and
    // ingroup_column show the same thing if there's only one group.
    $("td > div.inround").removeClass("hidden");
    $("td > div.insuper, td > div.nonracing-class").addClass("hidden");
    $(".insuper_column").removeClass("hidden");
    return $("tr[data-roundid='" + entry.roundid + "'][data-rankid='" + entry.rankid + "']");
  } else if (entry.kind == 'rank-round') {
    $("div[data-standings-key=" + entry.key + "]").removeClass('hidden');
    $("td > div.inround, td > div.insuper, td > div.nonracing-class").addClass("hidden");
    $(".insuper_column").addClass("hidden");
    $(".subgroup_column,.insubgroup_column").addClass('hidden');
    $(".inround_column").removeClass('hidden');
    return $("tr[data-roundid='" + entry.roundid + "'][data-rankid='" + entry.rankid + "']");
  } else if (entry.kind == 'agg-class') {
    $("td > div.inround, td > div.insuper, td > div.nonracing-class").addClass("hidden");
    $(".insuper_column").removeClass("hidden");
    return $("div.nonracing-class[data-agg-classid=" + entry.classid + "]")
      .removeClass('hidden')
      .closest('tr');
  }
}

// https://stackoverflow.com/questions/1318076/jquery-hasattr-checking-to-see-if-there-is-an-attribute-on-an-element
function hasAttr(jq, attrname) {
  return typeof jq.attr(attrname) !== typeof undefined &&
         jq.attr(attrname) !== false;
}

// selection is jquery of an <option> element from the view selector.
// There are four cases:
//
//  The option for "All", if present, has no data attributes.
//  The option for a non-racing aggregate class has data-aggregate=classid.
//  The option for a round has data-roundid.  If there's also a data-rankid present,
//    then only the rows for that rankid should be presented.
function standings_select_on_change(selection, supergroup_label) {
  $("tr").not(".headers").addClass('hidden');
  $("div[data-standings-key]").addClass('hidden');
  select_standings_by_catalog_entry(JSON.parse(selection.attr('data-catalog-entry')))
    .removeClass('hidden');
}
