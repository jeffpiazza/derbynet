// The scenes.php page mostly comprises div#previews, which holds a list of
// div.kiosk divs, one per kiosk name.  Each div.kiosk includes a <select>
// element to set the assigned page for that kiosk in tht scene.
//
function rebuild_kiosk_divs(all_scene_kiosk_names, all_kiosk_pages) {
  $("#previews").empty();
  for (var i = 0; i < all_scene_kiosk_names.length; ++i) {
    var sel = $("<select class='kdiv-select'/>").append($("<option>(Unspecified)</option>").attr('value', -1));
    for (var j = 0; j < all_kiosk_pages.length; ++j) {
      sel.append($("<option/>")
                 .attr('value', j)
                 .text(all_kiosk_pages[j].brief));
    }
    sel.on('change', on_change_assigned_page);

    var name = all_scene_kiosk_names[i];
    $("#previews").append($("<div class='kiosk'/>")
                          .attr('data-kiosk', name)
                          .append($("<h3/>").text(name))
                          .append($("<div class='page-div'/>")
                                  .append(sel))
                          .append($("<div class='block_buttons'></div>"))
                         );
    mobile_select(sel);
  }
}

// Rebuilds the <select/> element for picking the current scene
function rebuild_scenes_selector() {
  var all_scenes = g_all_scenes;
  var current_scene = g_current_scene;

  $("#scenes-select").empty();
  var first_selection = 0;
  for (var i = 0; i < all_scenes.length; ++i) {
    var scene = all_scenes[i];
    $("#scenes-select").append($("<option/>")
                               .attr('value', scene.sceneid)
                               .text(scene.name));
    if (scene.sceneid == current_scene) {
      first_selection = i;
    }
  }
  $("#scenes-select").append($("<option/>")
                             .attr('value', -1)
                             .text("(New scene)"));
  $("#scenes-select")
    .on('change', on_scene_change)
    .val(all_scenes.length > 0 ? all_scenes[first_selection].sceneid : -1)
    .trigger('change');
  if (all_scenes.length == 0) {
    on_new_scene();
  }
}

function rebuild_scenes_page() {
  rebuild_kiosk_divs(g_all_scene_kiosk_names, g_all_pages);
  rebuild_scenes_selector();
}


// Invoked when the user chooses to examine a different scene.
function on_scene_change() {
  $("div.wrap").remove();
  var sceneid = $("#scenes-select").val();
  g_current_scene = sceneid;
  // If there are no scenes defined, jump right into defining a new one.
  if (sceneid < 0) {
    on_new_scene();
    return;
  }

  var scene = g_all_scenes.find((s) => { return s.sceneid == sceneid; });

  // unspecified keeps track of the kiosk names for which no page is assigned in
  // this scene.
  var unspecified = g_all_scene_kiosk_names.slice(0, g_all_scene_kiosk_names.length);

  for (var j = 0; j < scene.kiosks.length; ++j) {
    unspecified.splice(unspecified.indexOf(scene.kiosks[j].kiosk_name), 1);
    var kdiv = $("div.kiosk")
        .filter((i, elt) => $(elt).attr('data-kiosk') == scene.kiosks[j].kiosk_name);
    show_selected_page_for_kiosk(kdiv, scene.kiosks[j].page, JSON.parse(scene.kiosks[j].parameters));

    // For currently-selected scene, update select to match the page
    // Mark the event "synthetic" to avoid trying to update the server or re-generate
    // param controls.
    var page = g_all_pages.findIndex((p) => { return p.full == scene.kiosks[j].page; });
    // We need the change event in order to update the displayed choice for the select,
    // but the 'synthetic' flag will tell on_change_assigned_page not to do any redrawing; we'll
    // do that ourselves, here.
    kdiv.find("select.kdiv-select")
      .val(page)  // Value of the select is the index into g_all_pages
      .trigger('change', /*synthetic*/true);
  }

  for (var i = 0; i < unspecified.length; ++i) {
    var kdiv = $("div.kiosk").filter(function(index, element) {
      return element.getAttribute('data-kiosk') == unspecified[i];
    });
    kdiv.find("select.kdiv-select").val("-1").trigger('change', /*synthetic*/true);
    update_kiosk_iframe(kdiv, false, false);
  }
}

// Test that the name of a new scene isn't empty and doesn't collide with
// another scene name
function valid_new_scene_name() {
  var v = $("#new_scene_name").val().toLowerCase();
  if (v.length == 0) {
    return false;
  }
  if (g_all_scenes.findIndex((s) => { return s.name.toLowerCase() == v; }) >= 0) {
    return false;
  }
  return true;
}
function on_change_new_scene_name() {
  $("#new_scene_modal input[type=submit]")
    .prop('disabled', !valid_new_scene_name());
}
$(function() { $("#new_scene_name").on('keyup mouseup', on_change_new_scene_name); });

// Clicking the "New Scene" button 
function on_new_scene() {
  $("#new_scene_name").val("");  // Clear the name field
  on_change_new_scene_name();
  show_modal("#new_scene_modal", $("#new_scene_name"), function(event) {
    close_modal("#new_scene_modal");
    if (valid_new_scene_name()) {
      var name = $("#new_scene_name").val();
      $.ajax('action.php',
             {type: 'POST',
              data: {action: 'scene.add',
                     name: name},
              success: function(data) {
                if (data.hasOwnProperty('scene-id')) {
                  var sceneid = data['scene-id'];
                  g_all_scenes.push({sceneid: sceneid,
                                     name: name,
                                     kiosks: []});
                  g_current_scene = sceneid;
                  rebuild_scenes_page();
                }
              }
             });
    }
    return false;
  });
}

// Invoked when a kiosk in a scene gets assigned a different page to display.
// 'synthetic' will be true if the change event was generated by on_scene_change
// in response to a change of the current scene rather than a user action on the
// specific kiosk page choice.
function on_change_assigned_page(event, synthetic = false) {
  if (!synthetic) {
    // The user assigned a new page for this kiosk; they haven't yet had the
    // opportunity to change the parameters to use with it.
    var kdiv = $(event.target).closest("div.kiosk");

    var val = $(event.target).val();
    if (val >= 0) {
      var full_page_name = g_all_pages[val].full;
    }

    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'scene.setkiosk',
                   sceneid: $("#scenes-select").val(),
                   kiosk_name: kdiv.attr('data-kiosk'),
                   page:  full_page_name},
            success: function(data) {
              g_all_scenes = data['all-scenes'];
              show_selected_page_for_kiosk(kdiv, full_page_name, {});
            }
           });
  }
}

function show_selected_page_for_kiosk(kdiv, full_page_name, parameters) {
  var deco_div = kdiv.find('div.block_buttons');
  deco_div.empty();

  if (full_page_name) {
    if (g_kiosk_page_handlers.hasOwnProperty(full_page_name)) {
      var handler = g_kiosk_page_handlers[full_page_name];
      if (handler.hasOwnProperty('decorate')) {
        handler.decorate(deco_div, parameters,
                         function(new_params) {
                           set_scene_kiosk_params(kdiv, full_page_name, new_params);
                           show_selected_page_for_kiosk(kdiv, full_page_name, new_params);
                         });
      }
    }
    update_kiosk_iframe(kdiv, full_page_name, JSON.stringify(parameters));
  } else {
    update_kiosk_iframe(kdiv, '', '');
  }
}

function update_kiosk_iframe(kdiv, full_page_name, param_string) {
  kdiv.find("div.wrap").remove();
  if (full_page_name) {
    kdiv.append($("<div class='wrap'></div>")
                .append($("<iframe></iframe>")
                        .attr('src', "kiosk.php?page="
                              + encodeURIComponent(full_page_name)
                              + "&parameters=" + encodeURIComponent(param_string))));
  }
}

function set_scene_kiosk_params(kdiv, full_page_name, params) {
  var param_string = JSON.stringify(params);
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'scene.setkiosk',
                 sceneid: g_current_scene,
                 kiosk_name: kdiv.attr('data-kiosk'),
                 page:  full_page_name,
                 params: param_string},
          success: function(data) {
            g_all_scenes = data['all-scenes'];
            // Repopulate the kiosk decoration with the new parameters
            show_selected_page_for_kiosk(kdiv, full_page_name, params);
          }
         });
}

function valid_new_kiosk_name() {
  var v = $("#new_kiosk_name").val().toLowerCase();
  if (v.length == 0) {
    return false;
  }
  if (g_all_scene_kiosk_names.findIndex((s) => { return s.toLowerCase() == v; }) >= 0) {
    return false;
  }
  return true;
}
function on_change_new_kiosk_name() {
  $("#new_kiosk_modal input[type=submit]")
    .prop('disabled', !valid_new_kiosk_name());
}
$(function() { $("#new_kiosk_name").on('keyup mouseup', on_change_new_kiosk_name); });

function on_add_kiosk() {
  $("#new_kiosk_name").val("");
  on_change_new_kiosk_name();
  show_modal("#new_kiosk_modal", $("#new_kiosk_name"), function(event) {
    close_modal("#new_kiosk_modal");
    if (valid_new_kiosk_name()) {
      g_all_scene_kiosk_names.push($("#new_kiosk_name").val());
      g_all_scene_kiosk_names.sort((e1, e2) => {
        if (e1 == "Main") return -1;
        if (e2 == "Main") return  1;
        if (e1 < e2) return -1;
        if (e1 > e1) return  1;
        return 0;
      });

      rebuild_scenes_page();
    }
    return false;
  });
}

function on_delete_scene() {
  var scene = g_all_scenes.find((s) => s.sceneid == g_current_scene);
  if (confirm("Really delete scene " + scene.name + "?")) {
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'scene.delete',
                   sceneid: g_current_scene},
            success: function(data) {
              var scene_index = g_all_scenes.findIndex((s) => s.sceneid == g_current_scene);
              if (scene_index >= 0) {  // Should always be true
                g_all_scenes.splice(scene_index, 1);
              }
              g_current_scene = '';
              rebuild_scenes_selector();
            }
           });
  }
}

$(function() { rebuild_scenes_page(); });

