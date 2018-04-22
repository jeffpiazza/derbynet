//////////////////////////////////////////////////////////////////////////
// Polling for kiosk dashboard
//////////////////////////////////////////////////////////////////////////
function poll_kiosk_all() {
  $.ajax(g_action_url,
         {type: 'GET',
          data: {query: 'poll.kiosk.all'},
          success: function(data) {
            setTimeout(poll_kiosk_all, 2000);
            generate_kiosk_control_group(parse_kiosk_pages(data),
                                         parse_kiosks(data));
            $("#kiosk_control_group").trigger("create");
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
// A function invoked with each new poll
// init_for_rebuild = function() {}
//
// Invoked if any kiosk is displaying this page
// init_found = function() {}
//
// Adds any controls desired for custom configuration
//    kiosk describes the kiosk: {name:, address:, last_contact:, assigned_page:, parameters:}
//    kiosk_select is the <div> to which the page handler should add any desired
//        configuration controls (e.g., a Configure button that activates a
//        modal dialog).
// configure = function(kiosk, kiosk_select) {}

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
    configure: function(kiosk, kiosk_select) {
      configure_class_ids(kiosk, kiosk_select);
    }
  },
  'kiosks/slideshow.kiosk': {
    configure: function(kiosk, kiosk_select) {
      configure_title_and_class_ids(kiosk, kiosk_select);
    }
  },
};

// Configuration function for parameters of {classids: [...]}
function configure_class_ids(kiosk, kiosk_select) {
  $('<input type="button" data-enhanced="true" value="Configure"/>')
    .on("click", /* selector */null, /* data: */kiosk,
        /* handler */ show_config_classes_modal)
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
function configure_title_and_class_ids(kiosk, kiosk_select) {
  $('<input type="button" data-enhanced="true" value="Configure"/>')
    .on("click", /* selector */null, /* data: */kiosk,
        /* handler */ show_config_title_and_classes_modal)
    .appendTo(kiosk_select);

  // TODO Show title
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

// pages: array of {brief:, path:}
// kiosks: array of {name:, address:, last_contact:, assigned_page:, parameters:}
function generate_kiosk_control_group(pages, kiosks) {
  var hash = 0;
  for (var i = 0; i < kiosks.length; ++i) {
    var kiosk = kiosks[i];
    hash = hash_string(hash, kiosk.name);
    hash = hash_string(hash, kiosk.address);
    hash = hash_string(hash, kiosk.last_contact);
    hash = hash_string(hash, kiosk.assigned_page);
    hash = hash_string(hash, JSON.stringify(kiosk.parameters));
  }
  if (hash != g_kiosk_hash) {
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
    g_kiosk_hash = hash;
  }
}

// Returns an array of entries, {brief:, path:}, describing each available kiosk page.
// (brief is the kiosk file name only, path is the full path to access it.)
function parse_kiosk_pages(data) {
  var kiosk_pages_xml = data.getElementsByTagName("kiosk-page");
  var kiosk_pages = new Array(kiosk_pages_xml.length);
  for (var i = 0; i < kiosk_pages_xml.length; ++i) {
    kiosk_pages[i] = {brief: kiosk_pages_xml[i].getAttribute('brief'),
                      path: kiosk_pages_xml[i].textContent};
  }
  return kiosk_pages;
}

// Returns an array of entries for each known kiosk currently connected to the server.
// {name:, address:, last_contact:, assigned_page:, parameters:}
function parse_kiosks(data) {
  var kiosks_xml = data.getElementsByTagName("kiosk");
  var kiosks = new Array(kiosks_xml.length);
  for (var i = 0; i < kiosks_xml.length; ++i) {
    var kiosk_xml = kiosks_xml[i];
    var param_string = kiosk_xml.getElementsByTagName("parameters")[0].textContent;
    kiosks[i] = {name: kiosk_xml.getElementsByTagName("name")[0].textContent,
                 address: kiosk_xml.getElementsByTagName("address")[0].textContent,
                 last_contact: kiosk_xml.getElementsByTagName("last_contact")[0].textContent,
                 assigned_page: kiosk_xml.getElementsByTagName("assigned_page")[0].textContent,
                 parameters: param_string ? JSON.parse(param_string) : {}
                };
  }
  return kiosks;
}

// Generates a block of controls for a single kiosk.
// index is just a sequential counter used for making unique control names.
// kiosk describes the kiosk's state: {name:, address:, last_contact:, assigned_page:, parameters:}
// pages is an array of {path:, brief:} objects, as produced by parse_kiosk_pages.
function generate_kiosk_control(index, kiosk, pages) {
  var kiosk_control = $("<div class=\"block_buttons control_group kiosk_control\"/>");

  var kiosk_ident = $("<div class='kiosk-ident'/>");
  kiosk_ident.append("<p>Kiosk <span class=\"kiosk_control_name\"></span>"
                     + " <span class=\"kiosk_control_address\"></span>"
                     + "</p>");
  kiosk_ident.find(".kiosk_control_name").text(kiosk.name);
  kiosk_ident.find(".kiosk_control_address").text(kiosk.address);
  kiosk_ident.find(".kiosk_control_address").toggleClass("de-emphasize", kiosk.name.length > 0);
  kiosk_ident.append('<input type="button" data-enhanced="true"'
                     + ' onclick="show_kiosk_naming_modal(\''
                     + kiosk.address.replace(/"/g, '&quot;').replace(/'/, "\\'")
                     + '\', \'' + kiosk.name.replace(/"/g, '&quot;').replace(/'/, "\\'")
                     + '\')"'
                     + ' value="Assign Name"/>');
  kiosk_ident.append("<p class=\"last_contact\">Last contact: " + kiosk.last_contact + "</p>");
  kiosk_ident.appendTo(kiosk_control);

  var kiosk_select = $("<div class='kiosk-select'/>");
  kiosk_select.append("<label for=\"kiosk-page-" + index + "\">Displaying:</label>");
  var sel = $("<select name=\"kiosk-page-" + index + "\"" 
              + " data-kiosk-address=\"" + kiosk.address + "\"" 
              + " onchange=\"handle_assign_kiosk_page_change(this)\""
              + "/>");
  for (var i = 0; i < pages.length; ++i) {
    opt = $("<option value=\"" + pages[i].path + "\">" + pages[i].brief + "</option>");
    if (kiosk.assigned_page == pages[i].path) {
      opt.prop("selected", true);
    }
    sel.append(opt);
  }

  sel.appendTo(kiosk_select);

  var kiosk_config_handler = g_kiosk_page_handlers[kiosk.assigned_page.replace("\\", "/")];
  if (kiosk_config_handler) {
    if ('init_found' in kiosk_config_handler) {
      kiosk_config_handler.init_found();
    }
    if ('configure' in kiosk_config_handler) {
      // TODO This doesn't update if just the parameter changes
      kiosk_config_handler.configure(kiosk, kiosk_select);
    }
  }

  kiosk_select.appendTo(kiosk_control);
  
  kiosk_control.appendTo("#kiosk_control_group");
}

//////////////////////////////////////////////////////////////////////////
// Controls for assigning pages to kiosks
//////////////////////////////////////////////////////////////////////////

// sel is the <select data-kiosk-address> input element
function handle_assign_kiosk_page_change(sel) {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'kiosk.assign',
                 address: sel.getAttribute('data-kiosk-address'),
                 page: sel.value},
          success: function(data) {
            generate_kiosk_control_group(parse_kiosk_pages(data),
                                         parse_kiosks(data));
            $("#kiosk_control_group").trigger("create");
          },
         });
}

function show_kiosk_naming_modal(address, name) {
  $("#kiosk_name_field").val(name);
  show_modal("#kiosk_modal", function(event) {
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
            generate_kiosk_control_group(parse_kiosk_pages(data),
                                         parse_kiosks(data));
            $("#kiosk_control_group").trigger("create");
          },
         });
}

//////////////////////////////////////////////////////////////////////////
// Controls for the standings display
//////////////////////////////////////////////////////////////////////////
function process_standings_reveal_result(data) {
  var reveal = data.documentElement.getElementsByTagName('standings');
  if (reveal.length > 0) {
    var pieces = reveal[0].textContent.split('-');
    var current_exposed = pieces[2];
    $("#current_exposed").text(current_exposed == '' ? 'all' : ('lowest ' + current_exposed));
    $(".standings-control .reveal h3").removeClass('hidden');
  }
}

$(function () {
  // TODO Disable buttons if there's no current roundid selection.
  $("select").on("change", function(event) {
    // The initial prompt, if present, is shown as a disabled option which
    // we can now remove.
    $(this).find("option:disabled").remove();
    var selection = $(this).find("option:selected");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {
              action: 'standings.reveal',
              roundid: selection.attr('data-roundid'),
              rankid: selection.attr('data-rankid'),
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
function show_config_classes_modal(event) {
  var kiosk = event.data;  // {name:, address:, assigned_page:, parameters: }
  $("#title_div").addClass('hidden');
  populate_classids(kiosk.parameters);
  show_modal("#config_classes_modal", function(event) {
    close_modal("#config_classes_modal");
    post_new_params(kiosk, {classids: compute_classids()});
    return false;
  });
}

function show_config_title_and_classes_modal(event) {
  show_config_classes_modal(event);
  var kiosk = event.data;  // {name:, address:, assigned_page:, parameters: }
  $("#title_div").removeClass('hidden');
  $("#title_text").val(kiosk.parameters.title);
  populate_classids(kiosk.parameters);
  show_modal("#config_classes_modal", function(event) {
    close_modal("#config_classes_modal");
    post_new_params(kiosk, {title: $("#title_text").val(), classids: compute_classids()});
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
  $("#config_classes_modal input[type='checkbox']").checkboxradio("refresh");
}

// Extract classids from user's choices in the UI
function compute_classids() {
  var any_unchecked = false;
  var classids = [];
  $("#config_classes_modal input[type='checkbox']").each(function() {
    if ($(this).prop("checked")) {
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
  console.log("post_new_params: new_params = " + JSON.stringify(new_params));  // TODO
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'kiosk.assign',
                 address: kiosk.address,
                 params: JSON.stringify(new_params)},
          success: function(data) {
            generate_kiosk_control_group(parse_kiosk_pages(data),
                                         parse_kiosks(data));
            $("#kiosk_control_group").trigger("create");
          },
         });
}
