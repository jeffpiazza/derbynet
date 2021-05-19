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
  $.ajax('action.php',
         {type: 'GET',
          data: {query: 'json.file.directory.nodata',
                 path: path},
          // Disable the alert from dashboard-ajax.js that would show up for
          // "unable to list directory"
          global: false,
          success: function(data) {
            if (!data.hasOwnProperty('path')) {
              return;
            }
            var path = data.path;
            $("#chooser_directory_path").val(path);

            if (data.hasOwnProperty('chosen')) {
              $("#chooser_file_name").val(data.chosen);
            }

            var supers = data['base-paths'];
            for (var i = 0; i < supers.length; ++i) {
              var text = supers[i].basename;
              if (text.length == 0) {
                text = "/";  // TODO Does Windows encounter this case?
              }
              $("<option/>").prependTo($("#chooser_directories_above"))
                .text(text).prop('value', supers[i].path);
            }

            var last = $("#chooser_directories_above option").filter(":last");
            last.attr("selected", "selected");
            mobile_select_refresh($("#chooser_directories_above"));

            var files = data.files;
            for (var i = 0; i < files.length; ++i) {
              var li = $("<li></li>").appendTo($("#chooser_directory_content"));
              if (files[i].isdir) {
                if (files[i].readable) {
                  li.text(files[i].file)
                    .addClass('icon-right button')
                    .on("click", null, path, chooser_visit_subdirectory);
                  if (files[i].writable) {
                    li.addClass('unwritable');
                  } else {
                    li.addClass('writable');
                  }
                } else {
                  // If it's not readable, then we can't explore, but show the icon-right icon anyway.
                  li.text(files[i].file)
                    .addClass("icon-right")
                    .addClass("unreadable");
                }
              } else {
                if (choosing_directory()) {
                  li.text(files[i].file).addClass("ui-li-static");
                } else {
                  li.text(files[i].file)
                    .addClass("button")
                    .on("click", null, path, chooser_select_existing);
                }
              }
            }
            $("#chooser_directory_content li:first").addClass("ui-first-child");
            $("#chooser_directory_content li:last").addClass("ui-last-child");

            $("#chooser_add_directory").off("click");
            $("#chooser_add_directory").on("click", null, path, chooser_show_add_directory_modal);
          },
         });
}

function chooser_visit_subdirectory(event) {
  repopulate_chooser_modal(event.data + $(event.target).text());
}

function chooser_visit_superdirectory(event) {
  repopulate_chooser_modal($(event.target).find(":selected").val());
}

function chooser_select_existing(event) {
  // event.data is the path
  // event.target.text is the file name
  $("#chooser_file_name").val($(event.target).text());
  return false;
}

function chooser_show_add_directory_modal(event) {
  var parent_path = event.data;
  show_tertiary_modal("#chooser_add_directory_modal", function() {
    chooser_close_add_directory_modal();
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'json.file.directory.new.nodata',
                   directory: parent_path + $("#chooser_new_directory_name").val()},
            success: function(data) {
              if (data.hasOwnProperty('path')) {
                repopulate_chooser_modal(data.path);
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
