'use strict';

// checkbox is an element to wrap
function flipswitch(checkbox) {
  checkbox = $(checkbox);
  // Since the checkbox per se won't be visible, any classes assigned to the
  // checkbox are assumed to be meant for the enclosing div.flipswitch,
  // including the flipswitch class that triggered the conversion.  Move the
  // class string accordingly.
  var classes = checkbox.attr('class');
  checkbox
    .removeClass(classes)
    .wrap('<div>')
    .before('<span class="on">On</span>'
            + '<span class="off">Off</span>')
    .on('change', function(event) {
      checkbox.parent().toggleClass('checked', checkbox.is(':checked'));
    })
    .parent()
    .addClass(classes)
    .toggleClass('checked', checkbox.is(':checked'))
    .on('click', function(event) {
      checkbox.trigger(event);
    });
}

$(function() {
  $("input.flipswitch[type=checkbox]").each(function(i, checkbox) { flipswitch(checkbox); });
});


function inspect_flipswitch() {
  console.log("Unchecked:");
  var div = $("div.flipswitch").not(".checked")[0];
  console.log({flipswitch: div.getBoundingClientRect(),
               on: $(div).find(".on")[0].getBoundingClientRect(),
               off: $(div).find(".off")[0].getBoundingClientRect()});
  console.log("Width: " +  div.getBoundingClientRect().width);
  console.log("Left gap: " +
              ($(div).find(".on")[0].getBoundingClientRect().left -
               div.getBoundingClientRect().left));
  console.log("Top gap: " +
              ($(div).find(".on")[0].getBoundingClientRect().top -
               div.getBoundingClientRect().top));
  console.log("Bottom gap: " +
              (div.getBoundingClientRect().bottom -
               $(div).find(".on")[0].getBoundingClientRect().bottom));

  console.log("Checked:");
  div = $("div.flipswitch.checked")[0];
  console.log({flipswitch: div.getBoundingClientRect(),
               on: $(div).find(".on")[0].getBoundingClientRect(),
               off: $(div).find(".off")[0].getBoundingClientRect()});
  console.log("Width: " +  div.getBoundingClientRect().width);
  console.log("Right gap: " +
              (div.getBoundingClientRect().right -
               $(div).find(".on")[0].getBoundingClientRect().right));
  console.log("Top gap: " +
              ($(div).find(".on")[0].getBoundingClientRect().top -
               div.getBoundingClientRect().top));
  console.log("Bottom gap: " +
              (div.getBoundingClientRect().bottom -
               $(div).find(".on")[0].getBoundingClientRect().bottom));
}
