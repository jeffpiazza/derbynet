// Pass roundid = false to show standings for the whole pack.
// Returns a string that selects the rows remaining to be revealed for the
// selected round.
function select_standings(roundid, group_name) {
  document.title =  group_name + ' Standings';
  $("div.banner_title").text(group_name + ' Standings');

  if (roundid === false) {
    // "All" case
    $("td > div.inround").addClass("hidden");
    $("td > div.insuper").removeClass("hidden");
    $("th.insuper,td.insuper").addClass("hidden");
    return "tr[data-for-supergroup='1']";
  } else {
    $("td > div.inround").removeClass("hidden");
    $("td > div.insuper").addClass("hidden");
    $("th.insuper,td.insuper").removeClass("hidden");
    return "tr[data-roundid='" + roundid + "']";
  }
}

function standings_select_on_change(selection, supergroup_label) {
  $("tr").not(".headers").addClass('hidden');
  if (typeof selection.attr('data-roundid') == typeof undefined ||
      selection.attr('data-roundid') === false) {
    $(select_standings(false, supergroup_label)).removeClass('hidden');
  } else {
    $(select_standings(selection.attr('data-roundid'), selection.text())).removeClass('hidden');
  }
}
