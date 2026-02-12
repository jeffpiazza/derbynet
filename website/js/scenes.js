// The scenes.php page mostly comprises a list of div.kiosk divs, one per kiosk
// name.  Each div.kiosk includes a <select> element to set the assigned page
// for that kiosk in tht scene.

function setup_kiosk_divs(all_scene_kiosk_names, all_kiosk_pages) {
  $("#previews").empty();
  for (var i = 0; i < all_scene_kiosk_names.length; ++i) {
    var sel = $("<select class='kdiv-select'/>").append($("<option>(Unspecified)</option>").attr('value', -1));
    for (var j = 0; j < all_kiosk_pages.length; ++j) {
      sel.append($("<option/>")
                 .attr('value', j)
                 .text(all_kiosk_pages[j].brief));
    }
    sel.on('change', on_page_change);

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

function setup_scenes_select_control(all_scenes, current_scene) {
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
    var deco_div = kdiv.find('div.block_buttons');
    // Remove any old param controls
    deco_div.empty();

    if (g_kiosk_page_handlers.hasOwnProperty(scene.kiosks[j].page)) {
      var handler = g_kiosk_page_handlers[scene.kiosks[j].page];
      if (handler.hasOwnProperty('decorate')) {
        var page_name = scene.kiosks[j].page;
        (function(kdiv) {
        handler.decorate(deco_div, JSON.parse(scene.kiosks[j].parameters),
                         function(params) {
                           set_scene_kiosk_params(kdiv, page_name, params);
                         });
        })(kdiv);
      }
    }

    // For currently-selected scene, update select to match the page
    // Mark the event "synthetic" to avoid trying to update the server or re-generate
    // param controls.
    var page = g_all_pages.findIndex((p) => { return p.full == scene.kiosks[j].page; });
    // We need the change event in order to update the displayed choice for the select,
    // but the 'synthetic' flag will tell on_page_change not to do any redrawing; we'll
    // do that ourselves, here.
    kdiv.find("select.kdiv-select")
      .val(page)  // Value of the select is the index into g_all_pages
      .trigger('change', /*synthetic*/true);

    update_kiosk_iframe(kdiv, scene.kiosks[j].page, scene.kiosks[j].parameters)
  }

  for (var i = 0; i < unspecified.length; ++i) {
    var kdiv = $("div.kiosk").filter(function(index, element) {
      return element.getAttribute('data-kiosk') == unspecified[i];
    });
    kdiv.find("select.kdiv-select").val("-1").trigger('change', /*synthetic*/true);
    update_kiosk_iframe(kdiv, false, false);
  }
}
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

function on_new_scene() {
  // Clearing the name of the current scene
  $("#new_scene_name").val("");
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
                  setup_scenes_select_control(g_all_scenes, g_current_scene);
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
function on_page_change(event, synthetic) {
  if (!synthetic) {
    var kdiv = $(event.target).closest("div.kiosk");
    var val = $(event.target).val();
    if (val >= 0) {
      var page = g_all_pages[val];
      update_kiosk_iframe(kdiv, page.full, '{}');
    } else {
      update_kiosk_iframe(kdiv, '', '');
    }

    var deco_div = kdiv.find('div.block_buttons');
    // Remove any old param controls
    deco_div.empty();

    if (page && g_kiosk_page_handlers.hasOwnProperty(page.full)) {
      var handler = g_kiosk_page_handlers[page.full];
      if (handler.hasOwnProperty('decorate')) {
        var page_name = page.full;
        handler.decorate(deco_div, {},
                         function(params) {
                           set_scene_kiosk_params(kdiv, page_name, params);
                         });
      }
    }
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'scene.setkiosk',
                   sceneid: $("#scenes-select").val(),
                   kiosk_name: kdiv.attr('data-kiosk'),
                   page:  page ? page.full : ''},
            success: function(data) {
              g_all_scenes = data['all-scenes'];
            }
           });
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
  update_kiosk_iframe(kdiv, full_page_name, param_string);
  kdiv.find('iframe').attr('src', "kiosk.php?page="
                           + encodeURIComponent(full_page_name)
                           + "&parameters=" + encodeURIComponent(param_string)
                          );
  $.ajax('action.php',
         {type: 'POST',
          data: {action: 'scene.setkiosk',
                 sceneid: g_current_scene,
                 kiosk_name: kdiv.attr('data-kiosk'),
                 page:  full_page_name,
                 params: param_string},
          success: function(data) {
            g_all_scenes = data['all-scenes'];
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

      setup_kiosk_divs(g_all_scene_kiosk_names, g_all_pages);
      setup_scenes_select_control(g_all_scenes, g_current_scene);
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
              setup_scenes_select_control(g_all_scenes, g_current_scene);
            }
           });
  }
}

$(function() {
  setup_kiosk_divs(g_all_scene_kiosk_names, g_all_pages);
  setup_scenes_select_control(g_all_scenes, g_current_scene);
});

