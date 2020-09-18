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
// There are four cases:
//
//  The option for "All", if present, has no data attributes.
//  The option for a non-racing aggregate class has data-aggregate=classid.
//  The option for a round has data-roundid.  If there's also a data-rankid present,
//    then only the rows for that rankid should be presented.
function standings_select_on_change(selection, supergroup_label) {
  $("tr").not(".headers").addClass('hidden');
  $("tr." + selection.attr('value')).removeClass('hidden');
  $("td.col0 div, .col-group,.col-ingroup, .col-subgroup,.col-insubgroup, .col-insuper, .col-inround").addClass('hidden');
  var presentation = {};

  presentation['ff'] = 'div.insuper, .col-group,.col-ingroup, .col-subgroup,.col-insubgroup';
  // TODO Aggregate racing group likely needs its own presentation
  // TODO Aggregate racing group should show organic group, but organic rounds shouldn't.
  // TODO For aggregate racing round, populate ingroup column with ... ?
  presentation['class'] = 'div.inround, .col-group,.col-subgroup,.col-insubgroup';
  // TODO For subgroup, maybe just adjust the column header name for col0 to be
  // "in <group-label>", and suppress .col-ingroup.
  presentation['subgroup'] = 'div.inround, .col-ingroup, .col-insubgroup, .col-insuper';
  presentation['round'] = 'div.inround, .col-subgroup,.col-insubgroup';
  // TODO For round-subgroup, maybe show .col-inround and suppress col0 altogether?
  presentation['round-subgroup'] = 'div.inround, .col-insubgroup';
  presentation['qual'] = '.col-group,.col-ingroup, .col-subgroup,.col-insubgroup';

  $(presentation[selection.attr('data-presentation')]).removeClass('hidden');
  $("div." + selection.attr('value')).removeClass('hidden');

  console.log('Key = ' + selection.attr('value'));
  console.log('Presentation = ' + selection.attr('data-presentation'));
}
