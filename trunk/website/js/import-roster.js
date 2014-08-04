// From http://stackoverflow.com/questions/1219860/html-encoding-in-javascript-jquery:
function htmlEncode( html ) {
    return document.createElement( 'a' ).appendChild( 
        document.createTextNode( html ) ).parentNode.innerHTML;
}

function htmlDecode( html ) {
    var a = document.createElement( 'a' ); a.innerHTML = html;
    return a.textContent;
}

function printTable(file) {  // a File object
    var reader = new FileReader();

    reader.onload = function(event) {
        var csv = event.target.result;
        try {
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
                html += '<tr data-row="' + (1 + parseInt(row)) + '"';
                if (first) {
                    html += ' class="header_row"';
                }
                html += '>';
                if (first) {
                    html += '<th class="outcome">Label?</th>\n';
                } else {
                    html += '<th class="outcome"/>\n';
                }
                for (var item in data[row]) {
                    html += '<td class="dim column' + item + '">' + htmlEncode(data[row][item]) + '</td>\n';
                }
                html += '</tr>\n';
                first = false;
            }
            $('#csv_content').html(html);
        } catch(err) {
            alert('Failure: ' + err);
        }
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
            $('.column' + $(this).attr('data-column')).removeClass('dim');

            if ($('.field.required').closest('[data-home]').length == 0) {
                $('#import_button').removeClass('hidden');
                $('#submit_message').addClass('hidden');
            } else {
                $('#import_button').addClass('hidden');
                $('#submit_message').removeClass('hidden');
            }

            if ($(ui.draggable[0]).attr('data-field') == 'classname') {
                collectClassNames($(this).attr('data-column'));
            }
        }
    });
}

// TODO: Something similar for ranks/subgroups
// TODO: action.php?query=classes, and compare existing classes
function collectClassNames(columnNumber) {
    $('#new_ranks').text('');
    var classnames = [];
    $('td.column' + columnNumber).each(function(index, elt) {
        // Don't count a header_row value
        if ($(elt).closest('.header_row').length == 0) {
            classnames[$(elt).text()] = 1;
        }
    });
    for (var cl in classnames) {
        $('#new_ranks').append(htmlEncode(cl) + '<br/>')
    }
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
    $('#submit_message').text('File import will be enabled when required fields are assigned.');

    // TODO: The .label_target elements are only created by
    // printTable(), above, and don't appear immediately within the
    // DOM.  Delaying like this seems to work, but isn't efficient or
    // guaranteed reliable.
    setTimeout(function() {
        makeDroppableLabelTarget($(".label_target"));
    }, 100);
}

function handle_import_one_row(row) {
    if ($('[data-row="' + row + '"]').length == 0) {
        return;
    }

    // Construct param array from what's in the table row
    var params = {action: 'import'};
    $('#csv_content').find('.label_target').each(function (index, label_target) {
        // label_target.attr('data-column')
        // label_target.find('.field').attr('data-field')

        var field = $(label_target).find('.field');

        if (field.length > 0) {
            params[field.attr('data-field')] =
                $('#csv_content [data-row="' + row + '"]' +
                  ' .column' + $(label_target).attr('data-column'))
                .text();
        }
    });

    $.ajax(g_checkin_action_url,
           {type: 'POST',
            data: params,
            success: function() {
                $('[data-row="' + row + '"] th').text('OK');
                handle_import_one_row(row + 1);
            },
            error: function(jqXHR, textStatus, errorThrown) {
                alert("Error: " + textStatus + " / " + errorThrown);
            }
           });
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

    $('#import_button').click(function() {
        handle_import_one_row(2);
    });

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

            if ($('.field.required').closest('[data-home]').length == 0) {
                $('#import_button').removeClass('hidden');
                $('#submit_message').addClass('hidden');
            } else {
                $('#import_button').addClass('hidden');
                $('#submit_message').removeClass('hidden');
            }
        }
    });

    $(".field").draggable({
          helper: 'clone',
          appendTo: 'body',
          cursorAt: { top: 0, left: 20 },
          opacity: 0.5,
          revert: 'invalid',
          });

    $(".file_target input").on('dragenter', function() {
        $(event.target).addClass("draghover");
    });
    $(".file_target input").on('dragleave', function() {
        $(event.target).removeClass("draghover");
    });
});
