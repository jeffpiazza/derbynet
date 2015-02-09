
// Requires dashboard-ajax.js
// Requires modal.js

// Pulled out as a separate function because it uses two fields
function write_mysql_connection_string() {
    $('#connection_string').val('mysql:' + 
                                'host=' + $('#mysql_host').val() + ';' +
                                'dbname=' + $('#mysql_dbname').val());
}

function hide_or_show_connection(jq, show) {
    if (show) {
        jq.slideDown();
        jq.removeClass('hidden');
    } else {
        jq.addClass('hidden');
        jq.slideUp();
    }
}

$(function () {
    $('input[name="connection_type"]').on('change', function() {
        $val = $('input[name="connection_type"]:checked').val();
        // $('#for_string_connection').toggleClass('hidden', $val != 'string');
        $('#connection_string').prop('disabled', $val != 'string');
        hide_or_show_connection($('#for_odbc_connection'), $val == 'odbc');
        hide_or_show_connection($('#for_mysql_connection'), $val == 'mysql');
        hide_or_show_connection($('#for_sqlite_connection'), $val == 'sqlite');
    });
    $('#odbc_dsn_name').on('keyup', function() {
        $('#connection_string').val('odbc:DSN=' + $(this).val() + ';Exclusive=NO');
    });
    $('#mysql_host').on('keyup', write_mysql_connection_string);
    $('#mysql_dbname').on('keyup', write_mysql_connection_string);
    $('#sqlite_path').on('keyup', function() {
        $('#connection_string').val('sqlite:' + $(this).val());
    });
});

function show_choose_database_modal() {
  show_modal("#choose_database_modal", function(event) {
      handle_choose_database();
      return false;
  });
}

function handle_choose_database() {
    close_modal("#choose_database_modal");
    $.ajax('setup-action.php',
           {type: 'POST',
            data: $("#choose_database_modal form").serialize(),
            success: function(data) {
                console.log("Success");
                console.log(data);
	            var success = data.documentElement.getElementsByTagName("success");
                if (success && success.length > 0) {
                    alert("Configuration successful!");
                    location.reload(true);
                }
            }
           });
}

function show_initialize_schema_modal() {
  show_modal("#initialize_schema_modal", function(event) {
      handle_initialize_schema();
      return false;
  });
}

function handle_initialize_schema() {
    close_modal("#initialize_schema_modal");
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'run-sql',
                   script: 'schema'},
            success: function(data) {
	            var success = data.documentElement.getElementsByTagName("success");
                if (success && success.length > 0) {
                    alert("Schema definition successful!");
                    location.reload(true);
                }
            },
           });
}

function show_update_schema_modal() {
  show_modal("#update_schema_modal", function(event) {
      handle_update_schema();
      return false;
  });
}

function handle_update_schema() {
    close_modal("#update_schema_modal");
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'run-sql',
                   script: 'update-schema'},
            success: function(data) {
	            var success = data.documentElement.getElementsByTagName("success");
                if (success && success.length > 0) {
                    alert("Schema update successful!");
                    location.reload(true);
                }
            },
           });
}
