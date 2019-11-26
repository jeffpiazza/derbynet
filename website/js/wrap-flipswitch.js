// Writing just the checkbox and then triggering the "create" event seems to
// work the first time, but subsequent appearances don't show the jquery-mobile
// embellishments.  So this method wraps a jquery checkbox input element with
// the materials to make it a flipswitch.
function wrap_flipswitch(checkbox) {
  return checkbox
    .attr('data-role', 'flipswitch')
    .attr('data-enhanced', 'true')
    .addClass('ui-flipswitch-input')
    .wrap('<div class="ui-flipswitch ui-shadow-inset'
          + ' ui-bar-inherit ui-corner-all'
          + ' aggregate-flipswitch">')
    .before('<a href="#" class="ui-flipswitch-on ui-btn'
            + ' ui-shadow ui-btn-inherit">On</a>'
            + '<span class="ui-flipswitch-off">Off</span>')
    .parent()
    .toggleClass('ui-flipswitch-active', checkbox.is(':checked'));
}
