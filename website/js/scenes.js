function setup_kiosk_previews(all_scene_kiosk_names, all_kiosk_pages) {
  $("#previews").empty();
  for (var i = 0; i < all_scene_kiosk_names.length; ++i) {
    var sel = $("<select/>").append($("<option>(Unspecified)</option>").attr('value', -1));
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
    .val(all_scenes[first_selection].sceneid)
    .trigger('change');
}


function on_scene_change() {
  $("div.wrap").remove();
  var sceneid = $("#scenes-select").val();
  g_current_scene = sceneid;
  if (sceneid < 0) {
    on_new_scene();
    return;
  }
  var unspecified = g_all_scene_kiosk_names.slice(0, g_all_scene_kiosk_names.length);
  for (var i = 0; i < g_all_scenes.length; ++i) {
    var scene = g_all_scenes[i];
    if (scene.sceneid == sceneid) {
      for (var j = 0; j < scene.kiosks.length; ++j) {
        unspecified.splice(unspecified.indexOf(scene.kiosks[j].kiosk_name), 1);
        // TODO Special characters
        var kdiv = $("div.kiosk[data-kiosk=" + scene.kiosks[j].kiosk_name + "]");

        // For currently-selected scene, update select to match the page
        for (var p = 0; p < g_all_pages.length; ++p) {
          if (g_all_pages[p].full == scene.kiosks[j].page) {
            kdiv.find("select")
              .val(p)
              .trigger('change', /*synthetic*/true);
            break;
          }
        }
      }
      break;
    }
  }

  for (var i = 0; i < unspecified.length; ++i) {
    var kdiv = $("div.kiosk[data-kiosk=" + unspecified[i] + "]");
    kdiv.find("select").val("-1").trigger('change', /*synthetic*/true);
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
  $("#scenes-select").val(0).trigger("change");
  $("#new_scene_name").val("");
  on_change_new_scene_name();
  show_modal("#new_scene_modal", $("#new_scene_name"), function(event) {
    close_modal("#new_scene_modal");
    if (valid_new_scene_name()) {
      var name = $("#new_scene_name").val();
      $.ajax('action.php',
             {type: 'POST',
              data: {action: 'scene.new',
                     name: name},
              success: function(data) {
                var sceneid = data.getElementsByTagName('scene');
                if (sceneid.length > 0) {
                  sceneid = sceneid[0].getAttribute('id');
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

function on_page_change(event, synthetic) {
  var kdiv = $(event.target).closest("div.kiosk");
  kdiv.find("div.wrap").remove();
  var val = $(event.target).val();
  var page = {brief: '', full: ''};
  if (val >= 0) {
    page = g_all_pages[val];
    kdiv.append($("<div class='wrap'></div>")
                .append($("<iframe></iframe>")
                        .attr('src', "kiosk.php?page="
                              + encodeURIComponent(page.full))));
  }
  if (!synthetic) {
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'scene.setkiosk',
                   sceneid: $("#scenes-select").val(),
                   kiosk_name: kdiv.attr('data-kiosk'),
                   page:  page.full}
           });
  }
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

      setup_kiosk_previews(g_all_scene_kiosk_names, g_all_pages);
      setup_scenes_select_control(g_all_scenes, g_current_scene);
    }
    return false;
  });
}

function on_delete_scene() {
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
            setup_scenes_select_control();
          }
         });
}

$(function() {
  setup_kiosk_previews(g_all_scene_kiosk_names, g_all_pages);
  setup_scenes_select_control(g_all_scenes, g_current_scene);
});

