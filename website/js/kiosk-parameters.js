//////////////////////////////////////////////////////////////////////////
// Handling for setting specific kiosk pages' parameters
//////////////////////////////////////////////////////////////////////////

// "decorate" functions add controls (most often a "Configure" button) to the
// passed-in div for the kiosk.

// Things like modal dialogs are defined in kiosk-dashboard.php, and each kiosk
// page requiring parameter handling registers a page handler here.  Ideally
// we'd prefer that each kiosk page be able to supply its own arbitrary
// configuration UI, but this implementation isn't nearly that modular.
//
// TODO Find a better way to express this in javascript, so that each function
// has a definition and there's no need for all the 'in' testing at the call
// sites.
//
// A kiosk page handler potentially defines:
//
// // A function invoked with each new poll
// init_for_rebuild = function() {}
//
// // Invoked if any kiosk is displaying this page
// init_found = function() {}
//
// // Adds any controls desired for custom configuration
// //    kiosk_div is the <div> to which the page handler should add any desired
// //        configuration controls (e.g., a Configure button that activates a
// //        modal dialog).
// //    parameters are the current parameter values for the kiosk page (if any)
// //    callback is a function to receive new parameter values (json) if they change
// decorate = function(kiosk_div, parameters, callback) {}

var g_kiosk_page_handlers = {};


// If there are any classids specified in the parameter, attach a <p> element to
// kiosk_div to describe the current setting.
function add_classids_description(parameters, kiosk_div) {
  if (parameters.hasOwnProperty('classids') && parameters.classids.length > 0) {
    var s = '';
    var classids = parameters.classids;
    for (var i = 0; i < classids.length; ++i) {
      s += ', ' + $("label[for='config-class-" + classids[i] + "']").text();
    }
    $('<p class="parameters"/>').text(s.substring(2)).appendTo(kiosk_div);
  }
}

// Update the UI controls to show the current set of classids
function populate_classids(parameters) {
  if (parameters.classids && parameters.classids.length > 0) {
    $("#config_classes_modal input[type='checkbox']").prop("checked", false);
    var classids = parameters.classids;
    for (var i = 0; i < classids.length; ++i) {
      $("#config-class-" + classids[i])
        .prop("checked", true);
    }
  } else {
    $("#config_classes_modal input[type='checkbox']").prop("checked", true);
  }
  $("#config_classes_modal input[type='checkbox']").each(function() {
    $(this).parent().toggleClass('checked', $(this).is(":checked"));
  });
  flipswitch_refresh($("#config_classes_modal input[type='checkbox']"));
}

// Extract classids from user's choices in the UI
function compute_classids() {
  var any_unchecked = false;
  var classids = [];
  $("#config_classes_modal input[type='checkbox']").each(function() {
    if ($(this).is(":checked")) {
      classids.push(parseInt($(this).data("classid")));
    } else {
      any_unchecked = true;
    }
  });

  if (!any_unchecked) {
    classids = [];
  }
  return classids;
}


//////////////////////////////////////////////////////////////////////////
// please-check-in
//////////////////////////////////////////////////////////////////////////

function decorate_please_check_in(kiosk_div, parameters, callback) {
  $('<input type="button" value="Configure"/>')
    .on("click", /* selector */null, /* data: */{parameters: parameters, callback: callback},
        /* handler */ show_config_please_check_in)
    .appendTo(kiosk_div);
  add_classids_description(parameters, kiosk_div);
}

// Uses #config_classes_modal with slideshow-specific stuff turned off
function show_config_please_check_in(event) {
  $("#slideshow_div").addClass('hidden');
  populate_classids(event.data.parameters);
  show_modal("#config_classes_modal", function(ev) {
    close_modal("#config_classes_modal");
    event.data.callback({classids: compute_classids()});
    return false;
  });
}

g_kiosk_page_handlers['kiosks/please-check-in.kiosk'] = {
  // Parameters: {classids: [ classid, ... ] }
  decorate: function(kiosk_div, parameters, callback) {
    decorate_please_check_in(kiosk_div, parameters, callback);
  }
};

//////////////////////////////////////////////////////////////////////////
// slideshow
//////////////////////////////////////////////////////////////////////////
// Configuration function for parameters of {title:, classids: [...]}
function decorate_slideshow(kiosk_div, parameters, callback) {
  $('<input type="button" value="Configure"/>')
    .on("click", /* selector */null, /* data: */{parameters: parameters, callback: callback},
        /* handler */ show_config_slideshow_modal)
    .appendTo(kiosk_div);

  add_classids_description(parameters, kiosk_div);
}

// Uses #config_classes_modal with extra #slideshow_div turned on.
function show_config_slideshow_modal(event) {
  console.log('event.data', event.data);
  var parameters = event.data.parameters;
  $("#slideshow_div").removeClass('hidden');
  $("#title_text").val(parameters.title);
  $("#slideshow_subdir").val(parameters.subdir || '');
  mobile_select_refresh($("#slideshow_subdir"))
  populate_classids(parameters);
  show_modal("#config_classes_modal", function(ev) {
    close_modal("#config_classes_modal");
    event.data.callback({title: $("#title_text").val(),
                         subdir: $("#slideshow_subdir").length ? $("#slideshow_subdir").val() : '',
                         classids: compute_classids()});
    return false;
  });
}

g_kiosk_page_handlers['kiosks/slideshow.kiosk'] = {
  // Parameters: {title: (string), subdir: (string), classids: [ classid, ... ] }
  decorate: function(kiosk_div, parameters, callback) {
    decorate_slideshow(kiosk_div, parameters, callback);
  }
};

//////////////////////////////////////////////////////////////////////////
// standings
//////////////////////////////////////////////////////////////////////////
g_kiosk_page_handlers['kiosks/standings.kiosk'] = {
    // The kiosk dashboard includes a div.standings-control that's used for
    // choosing a single roundid to display standings for.  The div is hidden
    // when the dashboard loads, or reloads(?), but exposed if one of the active
    // kiosks.
    init_for_rebuild: function() {
      $(".standings-control").addClass("hidden");
    },
    init_found: function() {
      $(".standings-control").removeClass("hidden");
    },
};

//////////////////////////////////////////////////////////////////////////
// award-presentations
//////////////////////////////////////////////////////////////////////////
g_kiosk_page_handlers['kiosks/award-presentations.kiosk'] = {
    // Parameters: { confetti: (bool) }
  decorate: function(kiosk_div, parameters, callback) {
    var k_id = 'k-' + Math.floor(Math.random() * 10000);
    if (!parameters.hasOwnProperty('confetti')) {
      parameters.confetti = true;
    }
    $("<input type='checkbox' class='flipswitch'/>")
      .attr('id', k_id)
      .prop('checked',  parameters.confetti)
      .on("change", /*selector*/null, /*data*/{parameters: parameters, callback: callback},
          /*handler*/function (ev) {
            var checked = $(event.target).is(':checked');
            event.data.callback({confetti: checked});
          })
        .appendTo(kiosk_div);
      $('<label for="' + k_id + '">Confetti</label>').appendTo(kiosk_div);
    }
};

//////////////////////////////////////////////////////////////////////////
// qrcode
//////////////////////////////////////////////////////////////////////////

function show_config_qrcode_modal(event) {
  var kiosk = event.data.parameters;  // parameters are { title, content }
  $("#qrcode-content").val(g_url + "/vote.php");
  if (parameters) {
    if (parameters.title) {
      $("#qrcode-title").val(parameters.title);
    }
    if (parameters.content) {
      $("#qrcode-content").val(parameters.content);
    }
  }
  show_modal("#config_qrcode_modal", function(ev) {
    close_modal("#config_qrcode_modal");
    event.data.callback({title: $("#qrcode-title").val(),
                         content: $("#qrcode-content").val()});
    return false;
  });
}

g_kiosk_page_handlers['kiosks/qrcode.kiosk'] = {
    // Parameters: { title: (string) content: (url) }
  decorate: function(kiosk_div, parameters, callback) {
    $('<input type="button" value="Configure"/>')
      .on("click", /* selector */null, /* data: */{parameters: parameters, callback: callback},
          /* handler */ show_config_qrcode_modal)
      .appendTo(kiosk_div);
    }
};
