// Animate the results-by-racer page so rows scroll up in a continuous loop.

function scroll_one() {
    // Each den round is in a separate tbody.  Each tbody starts with a
    // row identifying the den and round, followed by a row of column
    // header cells.
    //
    // Animate the table's top margin to scroll it within the enclosing div.scroll-bounding-rect
    // such that the top row scrolls out of sight.
    $(".main_table").animate(
        {'margin-top': -$(".main_table tr:first").outerHeight(/* include margin */true)}, 1000,
        // When that completes, move the now-hidden top row to an overflow tbody
        // at the bottom of the table.  If necessary, create the overflow tbody.
        function () {
            // If the top tbody elements are hidden, move them to the end of the table.
            // Be careful if everything's hidden.
            var last_real_tbody = $(".main_table tbody:last");
            if (last_real_tbody.attr('id') == 'overflow') {
              last_real_tbody = last_real_tbody.prev();
            }
            if (!last_real_tbody.hasClass("hidden")) {
              // Because last_real_tbody is not hidden, and comes before #overflow if there's an #overflow,
              // then this loop will stop with tbody:first not being #overflow.
              while ($(".main_table tbody:first").hasClass("hidden")) {
                // This is reversing the order, because last_real_tbody doesn't move
                var t = $(".main_table tbody:first");
                t.insertAfter(last_real_tbody);
                last_real_tbody = t;
              }
            }
            if ($("#overflow").length == 0) {
                $(".main_table").append('<tbody id="overflow"></tbody>');
                // Apply css class from the current top tbody
            }
            $("#overflow").attr('class', $(".main_table tbody:first").attr('class'))
                          .removeClass('being-scrolled');

            // Move top the row to the overflow tbody
            $(".main_table tr:first").appendTo("#overflow");
            $("#overflow tr").show();
            // Unscroll the table.  The goal is to do this at the same
            // instant as the top row's removal, so there's no visual "jump".
            // TODO for whatever reason, that's not quite working out.
            $(".main_table").css('margin-top', 0);
            
            // If the top tbody is now empty, move its attributes to the
            // overflow tbody, and delete the empty shell.
            var tbody = $(".main_table tbody:first");
            // The being-scrolled class prevents trying to hide a tbody whose
            // content has been partially moved to the overflow.
            tbody.addClass("being-scrolled");
            if (tbody.children().length == 0) {
              $.each(tbody[0].attributes, function(index, attr) {
                if (attr) {
                  if (attr.name != 'id') {
                    $("#overflow").attr(attr.name, attr.value);
                  }
                }
              });

              var id = tbody.attr('id');
              tbody.attr('id', '');
              // Changing the id attribute means we can no longer reference as $("#overflow")
              $("#overflow").attr('id', id).removeClass("being-scrolled");
              tbody.remove();
            }
        });
}

// TODO Don't scroll if we don't need to
setInterval(scroll_one, 2000);
