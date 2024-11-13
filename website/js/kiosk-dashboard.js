var g_unclaimed_scene_kiosk_names;

function on_new_kiosk_window_button() {
  window.open("kiosk.php?address=localhost+" + (100000 + Math.floor(Math.random() * 100000)), "_blank");
}
$(function() { $("#new_kiosk_window_button").on('click', on_new_kiosk_window_button); });


//////////////////////////////////////////////////////////////////////////
// Polling for kiosk dashboard
//////////////////////////////////////////////////////////////////////////
function poll_kiosk_all() {
  $.ajax(g_action_url,
         {type: 'GET',
          data: {query: 'poll.kiosk.all'},
          success: function(data) {
            if (data["cease"]) {
              window.location.href = '../index.php';
              return;  // Without setting the next timeout
            }
            setTimeout(poll_kiosk_all, 2000);
            process_polled_data(data);
          },
          error: function() {
            setTimeout(poll_kiosk_all, 2000);
          }
         });
}

$(function() { poll_kiosk_all(); });


//////////////////////////////////////////////////////////////////////////
// Parameter handling for specific pages
//////////////////////////////////////////////////////////////////////////

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
// //    kiosk describes the kiosk: {name:, address:, last_contact:, page:, parameters:}
// //    kiosk_select is the <div> to which the page handler should add any desired
// //        configuration controls (e.g., a Configure button that activates a
// //        modal dialog).
// decorate = function(kiosk, kiosk_select) {}

var g_kiosk_page_handlers = {
  'kiosks/standings.kiosk': {
    init_for_rebuild: function() {
      $(".standings-control").addClass("hidden");
    },
    init_found: function() {
      $(".standings-control").removeClass("hidden");
    },
  },
  'kiosks/please-check-in.kiosk': {
    // Parameters: {classids: [ classid, ... ] }
    decorate: function(kiosk, kiosk_select) {
      decorate_please_check_in(kiosk, kiosk_select);
    }
  },
  'kiosks/slideshow.kiosk': {
    // Parameters: {title: (string), subdir: (string), classids: [ classid, ... ] }
    decorate: function(kiosk, kiosk_select) {
      decorate_slideshow(kiosk, kiosk_select);
    }
  },
  'kiosks/award-presentations.kiosk': {
    // Parameters: { confetti: (bool) }
    decorate: function(kiosk, kiosk_select) {
      var k_id = 'k-' + kiosk.address.replace(/[:+]/g, '_');
      if (!kiosk.parameters.hasOwnProperty('confetti')) {
        kiosk.parameters.confetti = true;
      }
      $("<input type='checkbox' class='flipswitch'/>")
        .attr('id', k_id)
        .prop('checked',  kiosk.parameters.confetti)
        .on("change", /*selector*/null, /*data*/kiosk,
            /*handler*/function (event) {
              var checked = $(event.target).is(':checked');
              var kiosk = event.data;  // {name:, address:, page:, parameters: }
              post_new_params(kiosk, {confetti: checked});
            })
        .appendTo(kiosk_select);
      $('<label for="' + k_id + '">Confetti</label>').appendTo(kiosk_select);
    }
  },
  'kiosks/qrcode.kiosk': {
    // Parameters: { title: (string) content: (url) }
    decorate: function(kiosk, kiosk_select) {
      $('<input type="button" value="Configure"/>')
        .on("click", /* selector */null, /* data: */kiosk,
            /* handler */ show_config_qrcode_modal)
        .appendTo(kiosk_select);
    }
  },
};

function decorate_please_check_in(kiosk, kiosk_select) {
  $('<input type="button" value="Configure"/>')
    .on("click", /* selector */null, /* data: */kiosk,
        /* handler */ show_config_please_check_in)
    .appendTo(kiosk_select);
  add_classids_description(kiosk.parameters, kiosk_select);
}

// If there are any classids specified in the parameter, attach a <p> element to
// kiosk_select to describe the current setting.
function add_classids_description(parameters, kiosk_select) {
  if (parameters.classids && parameters.classids.length > 0) {
    var s = '';
    var classids = parameters.classids;
    for (var i = 0; i < classids.length; ++i) {
      s += ', ' + $("label[for='config-class-" + classids[i] + "']").text();
    }
    $('<p class="parameters"/>').text(s.substring(2)).appendTo(kiosk_select);
  }
}

// Configuration function for parameters of {title:, classids: [...]}
function decorate_slideshow(kiosk, kiosk_select) {
  $('<input type="button" value="Configure"/>')
    .on("click", /* selector */null, /* data: */kiosk,
        /* handler */ show_config_slideshow_modal)
    .appendTo(kiosk_select);

  add_classids_description(kiosk.parameters, kiosk_select);
}

//////////////////////////////////////////////////////////////////////////
// Construct dynamic elements for kiosk dashboard
//////////////////////////////////////////////////////////////////////////
// The polling rate for the page is relatively fast, and each time
// causes a rewrite of everything for all the kiosks.  If the user has
// opened the <select> element for choosing a page to present on a
// kiosk, rewriting the control will cause the user's action to be
// ignored.  To combat this, we only update the kiosk groups when
// there's a detectable change, and for that, we keep a hash of what
// the last state of the kiosk data was.
var g_kiosk_hash = -1;

function hash_string(hash, str) {
  for (i = 0; i < str.length; i++) {
	var ch = str.charCodeAt(i);
	hash = ((hash<<5)-hash)+ch;
	hash = hash & hash; // Convert to 32bit integer
  }
  return hash;
}

function process_polled_data(data) {
  if (data.hasOwnProperty('outcome') && data.outcome.summary != 'success') {
    console.log('Failure:', data.outcome);
    return;
  }
  var pages = data['kiosk-pages'];
  var kiosks = data['kiosks'];
  for (var i = 0; i < kiosks.length; ++i) {
    kiosks[i].parameters = kiosks[i].hasOwnProperty('parameters') && kiosks[i].parameters
      ? JSON.parse(kiosks[i].parameters) : {};
  }

  if (data.hasOwnProperty('current-scene')) {
    current_scene = data['current-scene'];
    if (current_scene != g_current_scene) {
      g_current_scene = current_scene;
      $("#scenes-select")
        .val(g_current_scene == '' ? -1 : g_current_scene)
        .trigger('change', /*synthetic*/true);
    }
  }

  var hash = 0;
  for (var i = 0; i < kiosks.length; ++i) {
    var kiosk = kiosks[i];
    hash = hash_string(hash, kiosk.name);
    hash = hash_string(hash, kiosk.address);
    hash = hash_string(hash, kiosk.last_contact);
    hash = hash_string(hash, kiosk.page);
    hash = hash_string(hash, JSON.stringify(kiosk.parameters));

    $("#kiosk_control_group .kiosk_control .reload p.reloading").eq(i)
      .toggleClass('hidden', !kiosk.reload);
  }
  if (hash != g_kiosk_hash) {
    g_kiosk_hash = hash;
    generate_kiosk_control_group(pages, kiosks);
    $("#kiosk_control_group").trigger("create");
    update_kiosk_names(kiosks);
  }
}

// pages: array of {brief:, path:}
// kiosks: array of {name:, address:, madlib: last_contact:, page:, parameters:}
function generate_kiosk_control_group(pages, kiosks) {
  for (var kiosk_page in g_kiosk_page_handlers) {
    var kiosk_page_handler = g_kiosk_page_handlers[kiosk_page];
    if ('init_for_rebuild' in kiosk_page_handler) {
      kiosk_page_handler.init_for_rebuild();
    }
  }
  $("#kiosk_control_group").empty();
  if (kiosks.length == 0) {
    $("#kiosk_control_group").append("<h3>No kiosks are presently registered.</h3>");
  } else {
    for (var i = 0; i < kiosks.length; ++i) {
      generate_kiosk_control(i, kiosks[i], pages);
    }
  }
}

function update_kiosk_names(kiosks) {
  var name_count = 0;
  for (var i = 0; i < kiosks.length; ++i) {
    if (kiosks[i].name != '') {
      ++name_count;
    }
  }
  $("#scenes-status-message")
    .text(name_count + " named kiosk" + (name_count == 1 ? "" : "s")
          + (name_count < kiosks.length ? " out of " + kiosks.length : ""))
    .toggleClass('red-text', name_count < kiosks.length);
}

// Generates a block of controls for a single kiosk.
// index is just a sequential counter used for making unique control names.
// kiosk describes the kiosk's state: {name:, address:, madlib:, last_contact:, age:, page:, parameters:}
// pages is an array of {path:, brief:} objects, as produced by parse_kiosk_pages.
function generate_kiosk_control(index, kiosk, pages) {
  var kiosk_control = $("<div class=\"block_buttons control_group kiosk_control\"/>");

  var kiosk_ident = $("<div class='kiosk-ident'/>");
  kiosk_ident.append("<p><span class=\"kiosk_control_name\"></span>"
                     + " <span class=\"kiosk_control_address\"></span>"
                     + "</p>");
  kiosk_ident.find(".kiosk_control_name").text(kiosk.name);
  kiosk_ident.find(".kiosk_control_address")
    .text(kiosk.madlib).toggleClass("de-emphasize", kiosk.name.length > 0);
  kiosk_ident.append('<input type="button"'
                     + ' onclick="show_kiosk_naming_modal(\''
                     + (kiosk.address
                        ? kiosk.address.toString().replace(/"/g, '&quot;').replace(/'/, "\\'")
                        : '')
                     + '\', \'' + kiosk.name.replace(/"/g, '&quot;').replace(/'/, "\\'")
                     + '\')"'
                     + ' value="Assign Name"/>');
  if (kiosk.age > 5) {
    kiosk_ident.append(
      $("<p class=\"last_contact\"/>").text("Last contact: " + kiosk.age + "s ago"));
  }
  kiosk_ident.appendTo(kiosk_control);

  kiosk_control.append($("<div/>").addClass('reload')
                       .append($("<input type='button' value='Reload' onclick='reload_kiosk(this)'/>")
                               .attr('kiosk-address', kiosk.address))
                       .append($("<p class='reloading'>Reloading</p>")
                               .toggleClass('hidden', !kiosk.reload)));

  var kiosk_select = $("<div class='kiosk-select'/>");
  kiosk_select.append("<label for=\"kiosk-page-" + index + "\">Displaying:</label>");
  var sel = $("<select name=\"kiosk-page-" + index + "\"" 
              + " data-kiosk-address=\"" + kiosk.address + "\"" 
              + " onchange=\"handle_assign_kiosk_page_change(this)\""
              + "/>");
  for (var i = 0; i < pages.length; ++i) {
    opt = $("<option value=\"" + pages[i].full + "\">" + pages[i].brief + "</option>");
    if (kiosk.page == pages[i].full) {
      opt.prop("selected", true);
    }
    sel.append(opt);
  }

  sel.appendTo(kiosk_select);
  mobile_select(sel);

  var kiosk_config_handler = g_kiosk_page_handlers[kiosk.page.replace("\\", "/")];
  if (kiosk_config_handler) {
    if ('init_found' in kiosk_config_handler) {
      kiosk_config_handler.init_found();
    }
    if ('decorate' in kiosk_config_handler) {
      // TODO This doesn't update if just the parameter changes
      kiosk_config_handler.decorate(kiosk, kiosk_select);
    }
  }

  kiosk_select.appendTo(kiosk_control);
  
  kiosk_control.appendTo("#kiosk_control_group");
}

//////////////////////////////////////////////////////////////////////////
// Controls for assigning pages to kiosks
//////////////////////////////////////////////////////////////////////////

function setup_scenes_select_control() {
  $("#scenes-select").empty();
  $("#scenes-select").append($("<option/>")
                             .attr('value', -1)
                             .html("&nbsp;"));
  for (var i = 0; i < g_all_scenes.length; ++i) {
    var scene = g_all_scenes[i];
    $("#scenes-select").append($("<option/>")
                               .attr('value', scene.sceneid)
                               .text(scene.name));
  }

  $("#scenes-select")
    .on('change', on_scene_change)
    .val(g_current_scene == '' ? -1 : g_current_scene)
    .trigger('change', /*synthetic*/true);
}
$(function() { setup_scenes_select_control(); });

// synthetic=false if user triggered scene change
function on_scene_change(event, synthetic) {
  if (!synthetic) {
    var val = $("#scenes-select").val();
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'scene.apply',
                   sceneid: val},
            success: function(data) {
              process_polled_data(data);
            },
           });
  }
}

// sel is the <select data-kiosk-address> input element
function handle_assign_kiosk_page_change(sel) {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'kiosk.assign',
                 address: sel.getAttribute('data-kiosk-address'),
                 page: sel.value},
          success: function(data) {
            process_polled_data(data);
          },
         });
}

function on_click_preferred_name(event) {
  console.log(event.target);
  console.log($(event.target).text());
  $("#kiosk_name_field").val($(event.target).text()).focus();
  return false;
}

function show_kiosk_naming_modal(address, name) {
  $("#kiosk_name_field").val(name);
  $("#preferred_kiosk_names").empty();
  for (var i = 0; i < g_all_scene_kiosk_names.length; ++i) {
    $("#preferred_kiosk_names")
      .append($("<div class='kiosk-name-prefill-button'/>").text(g_all_scene_kiosk_names[i])
              .on('click', on_click_preferred_name));
  }
  $("#preferred_kiosk_names").trigger('create');
  show_modal("#kiosk_modal", $("#kiosk_name_field"), function(event) {
    handle_name_kiosk(address, $("#kiosk_name_field").val());
    return false;
  });
}

function handle_name_kiosk(address, name) {
  close_modal("#kiosk_modal");
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'kiosk.assign',
                 address: address,
                 name: name},
          success: function(data) {
            process_polled_data(data);
          },
         });
}

//////////////////////////////////////////////////////////////////////////
// Controls for the standings display
//////////////////////////////////////////////////////////////////////////
function process_standings_reveal_result(data) {
  if (data.hasOwnProperty('exposed')) {
    var current_exposed = data.exposed;
    if (current_exposed === '') {
      $("#current_exposed").text('all');
      $("#current_unexposed").text('nothing');
    } else {
      var count = $("#standings-catalog option:selected").attr('data-count');
      if (current_exposed > count) {
        current_exposed = count;
      }
      $("#current_exposed").text('lowest ' + current_exposed);
      $("#current_unexposed").text('highest ' + (count - current_exposed));
    }
    $(".standings-control .reveal h3").removeClass('hidden');
  }
}

$(function () {
  // TODO Disable buttons if there's no current roundid selection.
  $("select#standings-catalog").on("change", function(event) {
    // The initial prompt, if present, is shown as a disabled option which
    // we can now remove.
    $(this).find("option:disabled").remove();
    var selection = $(this).find("option:selected");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {
              action: 'standings.reveal',
              'catalog-entry': selection.attr('data-catalog-entry')
            },
            success: function(data) {
              process_standings_reveal_result(data);
            }});
  });
});

function handle_reveal1() {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {
            action: 'standings.reveal',
            expose: '+1'
            },
          success: function(data) {
            process_standings_reveal_result(data);
          }});
}

function handle_reveal_all() {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {
            action: 'standings.reveal',
            expose: 'all'
            },
          success: function(data) {
            process_standings_reveal_result(data);
          }});
}

//////////////////////////////////////////////////////////////////////////
// Controls for the kiosk parameter for classes (please_check_in display)
//////////////////////////////////////////////////////////////////////////

// Uses #config_classes_modal with slideshow-specific stuff turned off
function show_config_please_check_in(event) {
  var kiosk = event.data;  // {name:, address:, page:, parameters: }
  $("#slideshow_div").addClass('hidden');
  populate_classids(kiosk.parameters);
  show_modal("#config_classes_modal", function(event) {
    close_modal("#config_classes_modal");
    post_new_params(kiosk, {classids: compute_classids()});
    return false;
  });
}

function show_config_qrcode_modal(event) {
  var kiosk = event.data;  // { title, content }
  $("#qrcode-content").val(g_url + "/vote.php");
  if (kiosk.parameters) {
    if (kiosk.parameters.title) {
      $("#qrcode-title").val(kiosk.parameters.title);
    }
    if (kiosk.parameters.content) {
      $("#qrcode-content").val(kiosk.parameters.content);
    }
  }
  show_modal("#config_qrcode_modal", function(event) {
    close_modal("#config_qrcode_modal");
    post_new_params(kiosk, {title: $("#qrcode-title").val(),
                            content: $("#qrcode-content").val()});
    return false;
  });
}

// Uses #config_classes_modal with extra #slideshow_div turned on.
function show_config_slideshow_modal(event) {
  var kiosk = event.data;  // {name:, address:, page:, parameters: }
  $("#slideshow_div").removeClass('hidden');
  $("#title_text").val(kiosk.parameters.title);
  $("#slideshow_subdir").val(kiosk.parameters.subdir || '');
  mobile_select_refresh($("#slideshow_subdir"))
  populate_classids(kiosk.parameters);
  show_modal("#config_classes_modal", function(event) {
    close_modal("#config_classes_modal");
    post_new_params(kiosk, {title: $("#title_text").val(),
                            subdir: $("#slideshow_subdir").length ? $("#slideshow_subdir").val() : '',
                            classids: compute_classids()});
    return false;
  });
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

function post_new_params(kiosk, new_params) {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'kiosk.assign',
                 address: kiosk.address,
                 params: JSON.stringify(new_params)},
          success: function(data) {
            console.log('After post_new_params:', data);
            process_polled_data(data);
          },
         });
}

function reload_kiosk(btn) {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'kiosk.reload',
                 address: $(btn).attr('kiosk-address')},
          success: function() {
            console.log('kiosk.reload for ' + $(btn).attr('kiosk-address'));
            $(btn).closest(".reload").find("p.reloading").removeClass('hidden');
          },
         });
}
