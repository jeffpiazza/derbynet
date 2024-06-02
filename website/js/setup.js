// Requires dashboard-ajax.js
// Requires modal.js

Dropzone.options.prefsDrop = {
  paramName: 'prefs',
  acceptedFiles: 'text/*,.conf,.pref',
  success: function(file, response) {
    this.removeFile(file);
    // console.log('success', response);
    report_success_json(response);
  },
};

// details = {
//            locked: true/false
//            database: {status:, details:},
//            schema: {status:, details:, button:}
//            purge: {nracers:, nawards:, nheats:, nresults:}
//            roster:
//            groups:
//            awards:
//            settings:
//            form_fields: {drivers:, radio:, sqlite_path:, odbc_dsn_name:}
function populate_details(details) {
  $("#database_step div.status_icon img").attr('src', 'img/status/' + details.database.status + '.png');
  $("#schema_step div.status_icon img").attr('src', 'img/status/' + details.schema.status + '.png');
  $("#roster_step div.status_icon img").attr('src', 'img/status/' + details.roster.status + '.png');
  $("#groups_step div.status_icon img").attr('src', 'img/status/' + details.groups.status + '.png');
  $("#awards_step div.status_icon img").attr('src', 'img/status/' + details.awards.status + '.png');
  $("#settings_step div.status_icon img").attr('src', 'img/status/' + details.settings.status + '.png');

  var disabled = (details.schema.button == 'disabled') || !details.database.writable 
  // $("#settings_step input[type='submit']").prop('disabled', disabled);
  $("#roster_step a.button_link, "
    + "#groups_step a.button_link, "
    + "#awards_step a.button_link, "
    + "#settings_step a.button_link").toggleClass('disabled', disabled);
  $("#purge_data_button")
    .prop('disabled', details.purge.nracers == 0 && details.purge.nawards == 0);

  $("#database_step").toggleClass('hidden', details.locked);
  
  if (details.schema.button == 'disabled' || !details.database.writable) {
    $("#schema_button").prop('disabled', true).attr('value', 'Initialize');
  } else if (details.schema.button == 'initialize') {
    $("#schema_button").prop('disabled', false).attr('value', 'Initialize')
      .off('click').on('click', function() { show_initialize_schema_modal(); });
  } else if (details.schema.button == 'update') {
    $("#schema_button").prop('disabled', false).attr('value', 'Update Schema')
      .off('click').on('click', function() { show_update_schema_modal(); });
  } else /* 're-initialize' */ {
    $("#schema_button").prop('disabled', false).attr('value', 'Re-Initialize')
      .off('click').on('click', function() { show_initialize_schema_modal(); });
  }

  $("#database_step div.step_details").html(details.database.details);
  $("#schema_details").html(details.schema.details);
  $("#roster_step div.step_details").html(details.roster.details);
  $("#groups_step div.step_details").html(details.groups.details);
  $("#awards_step div.step_details").html(details.awards.details);
  $("#settings_step div.step_details").html(details.settings.details);

  $("#offer_fake").toggleClass('hidden', details.roster.count > 0);
  $("#remind_fake").toggleClass('hidden', details.roster.fake == 0);
  
  function maybe_mark_driver_missing(driver, radio_id) {
    var driver_ok = ($.inArray(driver, details.form_fields.drivers) >= 0);
    if (driver_ok) {
      mobile_radio_enable($('#' + radio_id), true);
      $('label[for="' + radio_id + '"] span.missing_driver').html('');
    } else {
      mobile_radio_enable($('#' + radio_id), false);
      $('label[for="' + radio_id + '"] span.missing_driver').html('(Driver not loaded!)');
    }
  }
  
  maybe_mark_driver_missing('sqlite', 'sqlite_connection');
  maybe_mark_driver_missing('odbc', 'odbc_connection');
  
  $("#advanced_database_modal input[type='radio']").prop('checked', false);
  $("#" + details.form_fields.radio + "_connection").prop('checked', true);

  $(".connection_details").addClass("hidden");
  $("#for_" + details.form_fields.radio + "_connection").removeClass("hidden");

  $("#odbc_dsn_name").val(details.form_fields.odbc_dsn_name);
  $("#sqlite_path").val(details.form_fields.sqlite_path);
  $("#connection_string").val(details.form_fields.connection_string);

  $("#delete_race_results").prop('disabled', details.purge.nresults == 0);
  $("#delete_schedules").prop('disabled', details.purge.nheats == 0);
  $("#delete_racers").prop('disabled', details.purge.nracers == 0);
  $("#delete_awards").prop('disabled', details.purge.nawards == 0);

  $("#purge_nresults_span").text(details.purge.nresults);
  $("#purge_nschedules_span").text(details.purge.nheats);
  $("#purge_nracers_span").text(details.purge.nracers);
  $("#purge_nawards_span").text(details.purge.nawards);
}

function hide_reporting_box() {
  $("#reporting_box").removeClass('success failure').addClass('hidden').css('opacity', 100);
  $("#reporting_box_dismiss").addClass('hidden');
}

function report_in_progress() {
  $("#reporting_box_content").text("In Progress...");
  $("#reporting_box").removeClass('hidden success failure');
}

function report_success() {
  $("#reporting_box_content").text("Success");
  $("#reporting_box").addClass('success').removeClass('hidden');
  setTimeout(function() {
    $("#reporting_box").animate({opacity: 0}, 500,
                                function () { hide_reporting_box(); });
  }, 1000);
}

function report_failure(text) {
  $("#reporting_box_content").text(text);
  $("#reporting_box_dismiss").removeClass('hidden');
  $("#reporting_box").addClass('failure').removeClass('hidden');
  // Has to be explicitly cleared -- no timout to disappear
}

function report_success_json(data) {
  if (data.outcome.summary == 'success') {
    if (data.hasOwnProperty('details')) {
      populate_details(data.details);
    }
    report_success();
  } else {
    report_failure(data.outcome.description);
  }
}

function show_ezsetup_modal() {
  $('#ez_database_name').val('');
  $('#ez-old-nochoice').prop('selected', 'selected');
  mobile_select_refresh($('#ez_database_select'));
  show_modal("#ezsetup_modal", function(event) {
    handle_ezsetup_modal_submit();
    return false;
  });
}

function handle_ezsetup_modal_submit() {
  close_modal("#ezsetup_modal");
  var dbname = $("#ez_database_name").val();

  var myform = $("#ezsetup_modal form");
  var serialized = myform.serialize();

  report_in_progress();
  $.ajax('action.php',
         {type: 'POST',
          data: serialized, // action = setup.nodata
          success: function(data) {
            report_success_json(data);
          },
          error: function(event, jqXHR, ajaxSettings, thrownError) {
            report_failure(thrownError);
          }
         });
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

function update_sqlite_path() {
  $('#connection_string').val('sqlite:' + $("#sqlite_path").val());
}

function show_advanced_database_modal() {
  hide_modal("#ezsetup_modal");
  show_modal("#advanced_database_modal", function(event) {
    handle_advanced_database_modal_submit();
    return false;
  });

  // Merely setting the "checked" attribute doesn't trigger the "change"
  // handler that displays the right extra fields.
  $('input[name="connection_type"][checked]').click();
}

function handle_advanced_database_modal_submit() {
  close_modal("#advanced_database_modal");

  var myform = $("#advanced_database_modal form");
  // Serialize form data while temporarily enabling disabled inputs
  // (like #connection_string)
  var disabled = myform.find(':input:disabled').removeAttr('disabled');
  var serialized = myform.serialize();
  disabled.attr('disabled', 'disabled');

  report_in_progress();
  $.ajax('action.php',
         {type: 'POST',
          data: serialized, // action = setup.nodata
          success: function(data) {
            report_success_json(data);
          },
          error: function(event, jqXHR, ajaxSettings, thrownError) {
            report_failure(thrownError);
          }
         });
}

function show_purge_modal() {
  show_modal("#purge_modal", function(event) {
    close_modal("#purge_modal");
  });
}

function confirm_purge(purge) {
  var text = "some data";
  if (purge == 'results') {
    text = $("#purge_results_para").text();
  } else if (purge == 'schedules') {
    text = $("#purge_schedules_para").text();
  } else if (purge == 'racers') {
    text = $("#purge_racers_para").text();
  } else if (purge == 'awards') {
    text = $("#purge_awards_para").text();
  }
  
  $("#purge_operation").text(text);

  show_secondary_modal("#purge_confirmation_modal", function(event) {
    close_secondary_modal("#purge_confirmation_modal");
    close_modal("#purge_modal");
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'database.purge',
                   purge: purge},
            success: function(data) {
              report_success_json(data);
            },
            error: function(event, jqXHR, ajaxSettings, thrownError) {
              report_failure(thrownError);
            }
           });
  });
}

function show_initialize_schema_modal() {
  show_secondary_modal("#initialize_schema_modal", function(event) {
    handle_initialize_schema();
    return false;
  });
}

function handle_initialize_schema() {
  close_secondary_modal("#initialize_schema_modal");
  close_modal("#purge_modal");
  report_in_progress();
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'database.execute',
                 script: 'schema'},
          success: function(data) {
            report_success_json(data);
          },
          error: function(event, jqXHR, ajaxSettings, thrownError) {
            report_failure(thrownError);
          }
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
  report_in_progress();
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'database.execute',
                 script: 'update-schema'},
          success: function(data) {
            report_success_json(data);
          },
          error: function(event, jqXHR, ajaxSettings, thrownError) {
            report_failure(thrownError);
          }
         });
}


$(function () {
  $('input[name="connection_type"]').on('change', function() {
    val = $('input[name="connection_type"]:checked').val();
    // $('#for_string_connection').toggleClass('hidden', val != 'string');
    $('#connection_string').prop('disabled', val != 'string');
    hide_or_show_connection($('#for_odbc_connection'), val == 'odbc');
    hide_or_show_connection($('#for_sqlite_connection'), val == 'sqlite');
  });
  $('#odbc_dsn_name').on('keyup', function() {
    $('#connection_string').val('odbc:DSN=' + $(this).val() + ';Exclusive=NO');
  });
  $('#sqlite_path').on('keyup', update_sqlite_path);
});
