function printTable(file) {  // a File object
    var reader = new FileReader();

    reader.onload = function(event) {
        var csv = event.target.result;
        var data = $.csv.toArrays(csv);
        var html = '';
        var first = true;
        for (var row in data) {
            if (first) {
                // Add an extra row on top, to which we drag labels
                html += '<tr><th/>\n';
                for (var item in data[row]) {
                    html += '<th class="label_target" data-column="' + item + '">(Drag Label Here)</th>';
                }
                html += '</tr>\n';
            }
            html += '<tr data-row="' + row + '"';
            if (first) {
                html += ' class="first_row"';
            }
            html += '>';
            if (first) {
                html += '<th class="outcome">Label?</th>\n';
            } else {
                html += '<th class="outcome"/>\n';
            }
            for (var item in data[row]) {
                html += '<td class="dim column' + item + '">' + data[row][item] + '</td>\n';
            }
            html += '</tr>\n';
            first = false;
        }
        $('#csv_content').html(html);
    };

    reader.onerror = function() {
        alert('Unable to read ' + file.fileName);
    };

    reader.readAsText(file);
}

function dragOutOfLabelTarget(jq) {
    $('.column' + jq.attr('data-column')).addClass('dim');
    jq.removeClass('label_target_filled');

    makeDroppableLabelTarget(jq);

    // TODO: Calling jq.text will destroy the draggable too soon, so
    // we delay the action.
    setTimeout(function() {
        jq.text('(Drag Label Here)');
    }, 50);
}

function makeDroppableLabelTarget(jq) {
    jq.droppable({
        hoverClass: 'label_target_hover',
        drop: function (event, ui) {
            // $(this) is the .label_target jquery element
            $(this).text('');
            // If dragging out of a label_target, make the
            // label_target droppable again
            var old_target = $(ui.draggable[0]).closest(".label_target");
            if (old_target.length > 0) {
                dragOutOfLabelTarget(old_target);
            }
            $(this).append(ui.draggable[0]);
            // No longer droppable
            $(this).droppable("destroy");
            $(this).addClass('label_target_filled');
            // TODO: Draggable back to unused.
            $('.column' + $(this).attr('data-column')).removeClass('dim');
        }
    });
}

function onFileSelect(event) {
    var file = event.target.files[0];
    $('#meta').html('<span class="filename">' + file.name + '</span><br/>\n' +
                    'Type: ' + (file.type || '(not available)') + '<br/>\n' +
                    'Size: ' + file.size + ' bytes<br/>\n' +
                    'Last Modified: ' + (file.lastModifiedDate
                                         ? file.lastModifiedDate.toLocaleDateString()
                                         : '(not available)'));
    printTable(file);

    $(".fields").removeClass("hidden");
    $(".file_target").addClass("hidden");

    // TODO: The .label_target elements are only created by
    // printTable(), above, and don't appear immediately within the
    // DOM.  Delaying like this seems to work, but isn't efficient or
    // guaranteed reliable.
    setTimeout(function() {
        makeDroppableLabelTarget($(".label_target"));
    }, 100);
}

$(function() {
    if (window.File && window.FileReader) {
        $('#csv_file').bind('change', onFileSelect);
    } else {
        $('#meta').html('<h3>Please update your browser</h3>'
                        + '<p>Operation of this page depends on HTML5 features for local file handling' 
                        + ' that this browser doesn\'t support.  Nearly all browsers support these'
                        + ' features in their most recent versions.</p>');
    }

    $('.fields .target').droppable({
        hoverClass: 'fields_target_hover',
        drop: function (event, ui) {
            // event.target is the .target div
            // ui.draggable is the .field

            // If dragging out of a label_target, make the
            // label_target droppable again
            var old_target = $(ui.draggable[0]).closest(".label_target");
            if (old_target.length > 0) {
                dragOutOfLabelTarget(old_target);
            }

            $(this).find('[data-home="' + $(ui.draggable[0]).attr('data-field') + '"]')
                .append(ui.draggable[0]);
        }
    });

    $(".field").draggable({
          helper: 'clone',
          appendTo: 'body',
          cursorAt: { top: 0, left: 20 },
          opacity: 0.5,
          revert: 'invalid',
          });
});
