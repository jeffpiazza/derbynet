
function loadExportedCsv(file) {
    var reader = new FileReader();
    var fields = ['class', 'round', 'heat', 'lane', 'firstname', 'lastname', 'carnumber',
                  'finishtime', 'finishplace', 'completed'];

    reader.onload = function(event) {
        var csv = event.target.result;
        try {
            var data = $.csv.toArrays(csv);
            var params = {action: 'import-one-result'};
            var first = true;
            for (var row in data) {
                if (first) {
                    first = false;
                } else {
                    for (var item in data[row]) {
                        params[fields[item]] = data[row][item];
                    }
                    var result = $.ajax(g_action_url,
                                        {type: 'POST',
                                         async: false,
                                         data: params}).responseText;
                    console.log(result);
                }
            }
        } catch(err) {
            alert('Failure: ' + err);
        }
    };

    reader.onerror = function() {
        alert('Unable to read ' + file.fileName);
    };

    reader.readAsText(file, 'utf-8');
}


$(function() {
    if (window.File && window.FileReader) {
    } else {
        $('#meta').html('<h3>Please update your browser</h3>'
                        + '<p>Operation of this page depends on HTML5 features for local file handling' 
                        + ' that this browser doesn\'t support.  Nearly all browsers support these'
                        + ' features in their most recent versions.</p>');
    }

    $('#import_button').click(function() {
        loadExportedCsv($("#csv_file").prop('files')[0]);
        $("#submit_message").text("Import successful!");
        $("#submit_message").css('color', '#008000');
    });

    $(".file_target input").on('dragenter', function() {
        $(event.target).addClass("draghover");
    });
    $(".file_target input").on('dragleave', function() {
        $(event.target).removeClass("draghover");
    });
});
