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
            if ($("#overflow").length == 0) {
                $(".main_table").append('<tbody id="overflow"></tbody>');
                // Apply css class from the current top tbody
                $("#overflow").attr('class', $(".main_table tbody:first").attr('class'));
            }

            // Move top the row to the overflow tbody
            $(".main_table tr:first").appendTo("#overflow");
            $("#overflow tr").show();
            // Unscroll the table.  The goal is to do this at the same
            // instant as the top row's removal, so there's no visual "jump".
            // TODO for whatever reason, that's not quite working out.
            $(".main_table").css('margin-top', 0);
            
            // If the top tbody is now empty, move its id to the overflow tbody,
            // and delete the empty shell.
            var tbody = $(".main_table tbody:first");
            if (tbody.children().length == 0) {
                var id = tbody.attr('id');
                tbody.attr('id', '');
                $("#overflow").attr('id', id);
                tbody.remove();
            }
        });
}

$(setInterval(scroll_one, 2000));
