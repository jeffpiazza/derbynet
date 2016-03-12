$(function() {
    $("#chooser_directories_above").on("change", chooser_visit_superdirectory);
});

// What mode of operation are we using the file chooser for?  Possible values are:
//
// directory - Pick a directory for writing results
// file.out - Pick a new or existing file for writing outputs
// file.in - Pick an existing file for reading
//
// TODO file.in isn't implemented (not yet needed)
var g_file_chooser_mode = 'directory';

function choosing_directory() { return g_file_chooser_mode == 'directory'; }
function choosing_output_file() { return g_file_chooser_mode == 'file.out'; }
function choosing_file() { return choosing_output_file(); }

function repopulate_chooser_modal(path) {
    $("#chooser_directories_above").empty();
    $("#chooser_directory_content").empty();
    $.ajax(g_action_url,
           {type: 'GET',
            data: {query: 'directory',
                   path: path},
            success: function(data) {
                var root = data.getElementsByTagName("directory");
                if (root.length == 0) {
                    return;
                }
                root = root[0];
                var path = root.getElementsByTagName("path");
                if (path.length == 0) {
                    return;
                }
                path = path[0].getAttribute("realpath"); // Includes trailing directory separator
                $("#chooser_directory_path").val(path);

                var chosen = root.getElementsByTagName("chosen");
                if (chosen.length > 0) {
                    $("#chooser_file_name").val(chosen[0].textContent);
                }

                var supers = root.getElementsByTagName("base");
                for (var i = 0; i < supers.length; ++i) {
                    var text = supers[i].textContent;
                    if (text.length == 0) {
                        text = "/";  // TODO Does Windows encounter this case?
                    }
                    $("<option/>").prependTo($("#chooser_directories_above"))
                        .text(text).prop('value', supers[i].getAttribute('path'));
                }

                var last = $("#chooser_directories_above option").filter(":last");
                last.attr("selected", "selected");
                // jQuery Mobile makes the ...-button element from an existing element
                $("#chooser_directories_above-button span").text(last.text());

                // <file readable writable directory> xyz </file>
                var files = data.getElementsByTagName("file");
                for (var i = 0; i < files.length; ++i) {
                    var li = $("<li></li>").appendTo($("#chooser_directory_content"));
                    if (files[i].getAttribute("directory") == "1") {
                        if (files[i].getAttribute("readable") == "1") {
                            li.append("<a/>");
                            li.find("a").text(files[i].textContent)
                            .addClass("ui-btn ui-btn-icon-right ui-icon-carat-r")
                            .on("click", null, path, chooser_visit_subdirectory);
                            if (files[i].getAttribute("writable") == "0") {
                                li.find("a").addClass("unwritable");
                            } else {
                                li.find("a").addClass("writable");
                            }
                        } else {
                            // If it's not readable, then we can't explore, but show the icon-right icon anyway.
                            li.text(files[i].textContent)
                                .addClass("ui-li-static ui-btn-icon-right ui-icon-carat-r")
                                .addClass("unreadable");
                        }
                    } else {  // Ordinary file, not a directory
                        if (choosing_directory()) {
                            li.text(files[i].textContent).addClass("ui-li-static");
                        } else {
                            li.append("<a/>");
                            li.find("a").text(files[i].textContent)
                                .addClass("ui-btn")
                                .on("click", null, path, chooser_select_existing);
                        }
                    }
                }

                $("#chooser_modal form").enhanceWithin();
                $("#chooser_directory_content li:first").addClass("ui-first-child");
                $("#chooser_directory_content li:last").addClass("ui-last-child");

                $("#chooser_add_directory").off("click");
                $("#chooser_add_directory").on("click", null, path, chooser_show_add_directory_modal);
            },
           });
}

function chooser_visit_subdirectory(event) {
    repopulate_chooser_modal(event.data + event.target.text);
}

function chooser_visit_superdirectory(event) {
    repopulate_chooser_modal($(event.target).find(":selected").val());
}

function chooser_select_existing(event) {
    // event.data is the path
    // event.target.text is the file name
    $("#chooser_file_name").val(event.target.text);
    return false;
}

function chooser_show_add_directory_modal(event) {
    var parent_path = event.data;
    show_tertiary_modal("#chooser_add_directory_modal", function() {
        chooser_close_add_directory_modal();
        $.ajax(g_action_url,
               {type: 'POST',
                data: {action: 'make-directory',
                       directory: parent_path + $("#chooser_new_directory_name").val()},
                success: function(data) {
                    var path = data.getElementsByTagName("path");
                    if (path.length > 0) {
                        repopulate_chooser_modal(path[0].textContent);
                    }
                }
               });
        return false;
    });
}

function chooser_close_add_directory_modal() {
    close_tertiary_modal("#chooser_add_directory_modal");
}

function chooser_show_modal(path, on_choose) {
    $("#chooser_file_name").parent(".ui-input-text")
        .toggleClass("hidden", choosing_directory());

    repopulate_chooser_modal(path);
    show_secondary_modal("#chooser_modal", function() {
        close_chooser_modal();
        on_choose();
        return false;
    });
}

function show_choose_directory_modal(path, dir_path_callback) {
    g_file_chooser_mode = 'directory';
    chooser_show_modal(path, function() {
        dir_path_callback($("#chooser_directory_path").val());
    });
    return false;
}

// path is directory to which chooser will be pointed, filename the name of the file within that directory.
// file_path_callback gets called with the full path chosen.
function show_choose_file_modal(path, filename, file_path_callback) {
    g_file_chooser_mode = 'file.out';
    $("#chooser_file_name").val(filename);
    chooser_show_modal(path, function() { 
        // Hidden input #chooser_directory_path's value always includes trailing
        // directory separator
        file_path_callback($("#chooser_directory_path").val() + $("#chooser_file_name").val());
    });
    return false;
}

function close_chooser_modal() {
  close_secondary_modal("#chooser_modal");
}
