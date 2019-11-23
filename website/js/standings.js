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

// Pass roundid = false to show standings for the whole pack.  Returns a jquery
// of the rows remaining to be revealed for the selected round.  (Rows are
// revealed immediately in standings page, progressively in kiosk.)
//
// if roundid == false ("All" standings), hide inround, show insuper
//          hide insuper columns (?), show for-supergroup rows.
// otherwise: show inround, hide insuper text, show insuper columns
function select_standings(roundid, rankid, group_name) {
  document.title =  group_name + ' Standings';
  $("div.banner_title").text(group_name + ' Standings');

  if (roundid === false) {
    // "All" case
    $("td > div.inround, td > div.nonracing-class").addClass("hidden");
    $("td > div.insuper").removeClass("hidden");
    $("th.insuper,td.insuper").addClass("hidden");
    return $("tr[data-for-supergroup='1']");
  } else {
    $("td > div.inround").removeClass("hidden");
    $("td > div.insuper, td > div.nonracing-class").addClass("hidden");
    $("th.insuper,td.insuper").removeClass("hidden");
    if (rankid === false) {
      return $("tr[data-roundid='" + roundid + "']");
    } else {
      return $("tr[data-roundid='" + roundid + "'][data-rankid='" + rankid + "']");
    }
  }
}

function select_aggregate_standings(aggregate_classid, group_name) {
  document.title =  group_name + ' Standings';
  $("div.banner_title").text(group_name + ' Standings');

  $("td > div.inround, td > div.insuper, td > div.nonracing-class").addClass("hidden");
    $("th.insuper,td.insuper").removeClass("hidden");
  return $("div.nonracing-class[data-agg-classid=" + aggregate_classid + "]").removeClass('hidden')
    .closest('tr');
}

// Currently called only from standings-kiosk.js' poller.
//
// TODO select#view-selector should ideally call this method on change, and then
// we could remove select_standings and select_aggregate_standings as separate
// functions.
//
// entry has kind, key, and name, and, depending on kind, maybe roundid, rankid,
// and/or classid.
function select_standings_by_catalog_entry(entry) {
  if (entry.kind == 'supergroup') {
    return select_standings(false, false, entry.name);
  } else if (entry.kind == 'class' || entry.kind == 'round') {
    return select_standings(entry.roundid, false, entry.name)
  } else if (entry.kind == 'rank') {
    return select_standings(entry.roundid, entry.rankid, entry.name);
  } else if (entry.kind == 'agg-class') {
    return select_aggregate_standings(entry.classid, entry.name);
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

  if (hasAttr(selection, 'data-aggregate')) {
    select_aggregate_standings(selection.attr('data-aggregate'), selection.text()).removeClass();
  } else if (!hasAttr(selection, 'data-roundid')) {
    // Show "All" standings
    select_standings(false, false, supergroup_label).removeClass('hidden');
  } else if (!hasAttr(selection, 'data-rankid')) {
    // One round (and class standings)
    select_standings(selection.attr('data-roundid'), false, selection.text()).removeClass('hidden');
  } else {
    // One rank within one roundid
    select_standings(selection.attr('data-roundid'), selection.attr('data-rankid'), selection.text()).removeClass('hidden');
  }
}
