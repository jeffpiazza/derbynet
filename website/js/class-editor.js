// Requires dashboard-ajax.js

// Note that close_edit_all_classes_modal returns to setup.php upon dismissal of the
// modal dialog.

function show_edit_all_classes_modal() {
    reload_class_list();
    show_modal("#edit_all_classes_modal", function () { 
        return false;
    });
}

function close_edit_all_classes_modal() {
    close_modal("#edit_all_classes_modal");
    window.location='setup.php';
}

function show_add_class_modal() {
    show_secondary_modal("#add_class_modal", function () {
        close_add_class_modal();
        $.ajax(g_action_url,
               {type: 'POST',
                data: $('#add_class_modal form').serialize(),
                success: function (data) {
                    reload_class_list();
                }});
        
        return false;
    });
}

function close_add_class_modal() {
    close_secondary_modal("#add_class_modal");
}

// Show only the ul of ranks for this class, and only if we're using ranks in
// the first place.
function hide_ranks_except(classid) {
  $("#ranks_container ul").addClass("hidden");
  $("#ranks_container ul[data-classid=" + classid + "]").removeClass("hidden");
}

function edit_one_class(who) {
  var list_item = $(who).parent("li");
  var classid = list_item.attr('data-classid');

  // list_item is an <li> from #edit_all_classes_modal.  We expect it to have a
  // .class-name span child, plus data attributes for classid, count, and nrounds.
  
  $("#edit_class_name").val(list_item.find('.class-name').text());
  $("#edit_class_name").attr('data-classid', classid);

  hide_ranks_except(classid);

  // The "completed rounds" message appears only if there are no native racers,
  // but there are some completed rounds that prevent offering deletion of the
  // class.
  var count = list_item.attr('data-count');
  var nrounds = list_item.attr('data-nrounds');
  $("#completed_rounds_extension").toggleClass('hidden', !(count == 0 && nrounds != 0));
  $("#completed_rounds_count").text(nrounds);

  $("#delete_class_extension").toggleClass('hidden', !(count == 0 && nrounds == 0));
  show_edit_one_class_modal(list_item);
}

function show_edit_one_class_modal(list_item) {
    show_secondary_modal("#edit_one_class_modal", function () {
        $.ajax(g_action_url,
               {type: 'POST',
                data: {action: 'class.edit',
                       classid: list_item.attr('data-classid'),
                       name: $("#edit_class_name").val()},
                success: function () {
                    reload_class_list();
                }});

        close_edit_one_class_modal();
        return false;
    });
}

function close_edit_one_class_modal() {
    close_secondary_modal("#edit_one_class_modal");
}

function handle_delete_class() {
  close_edit_one_class_modal();
  if (confirm('Really delete ' + group_label_lc()
              + ' "' + $('#edit_one_class_modal input[name="name"]').val() + '"?')) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'class.delete',
                   classid: $("#edit_class_name").attr('data-classid')
                  },
            success: function(data) {
              repopulate_class_list(data);
            }
           });
  }
  return false;
}

function show_add_rank_modal() {
  var classid = $("#edit_class_name").attr('data-classid');
  $("#add_rank_modal input[name='classid']").val(classid);
  show_tertiary_modal("#add_rank_modal", function() {
    close_add_rank_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data: $("#add_rank_modal form").serialize(),
            success: function(data) {
              repopulate_class_list(data);
              hide_ranks_except(classid);
            }});
    return false;
  });
}

function close_add_rank_modal() {
  close_tertiary_modal("#add_rank_modal");
}

function edit_one_rank(who) {
  var list_item = $(who).parent("li");
  $("#edit_rank_name").val(list_item.find('.rank-name').text());
  $("#edit_rank_name").attr('data-rankid', list_item.attr('data-rankid'));

  // True if we're the only rank for this class:
  var only_rank = list_item.parent("ul").find("li").length == 1;
  var count = list_item.attr('data-count');
  $("#delete_rank_extension").toggleClass('hidden', only_rank || (count > 0));
  show_edit_one_rank_modal(list_item);
}

function show_edit_one_rank_modal(list_item) {
  var classid = list_item.closest("ul").attr('data-classid');
  $("#edit_rank_name").attr('data-classid', classid);
  show_tertiary_modal("#edit_one_rank_modal", function() {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'rank.edit',
                   rankid: list_item.attr('data-rankid'),
                   name: $("#edit_rank_name").val()},
            success: function(data) {
              repopulate_class_list(data);
              hide_ranks_except(classid);
            }});

    close_edit_one_rank_modal();
    return false;
  });
}

function close_edit_one_rank_modal() {
  close_tertiary_modal("#edit_one_rank_modal");
}

function handle_delete_rank() {
  var classid = $("#edit_rank_name").attr('data-classid');
  close_edit_one_rank_modal();
  if (confirm('Really delete ' + subgroup_label_lc()
              + ' "' + $("#edit_rank_name").val() + '"?')) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'rank.delete',
                   rankid: $("#edit_rank_name").attr('data-rankid')
                  },
            success: function(data) {
              repopulate_class_list(data);
              hide_ranks_except(classid);
            }
           });
  }
  return false;
}

function reload_class_list() {
    $.ajax(g_action_url,
           {type: 'GET',
            data: {query: 'class.list'},
            success: function(data) {
                repopulate_class_list(data);
            }});
}

// For XML <class> element cl, populate a list of ranks within the class in the
// edit_one_class modal.
function populate_rank_list(cl) {
  $("#edit_ranks_extension").removeClass('hidden');
  var classid = cl.getAttribute('classid');
  var rank_ul = $("<ul data-role='listview' data-split-icon='gear'></ul>").appendTo("#ranks_container");
  rank_ul.attr('data-classid', cl.getAttribute('classid'));

  var ranks = cl.getElementsByTagName("rank");
  for (var ri = 0; ri < ranks.length; ++ri) {
    var rank = ranks[ri];
    var rank_li = $("<li class='ui-li-has-alt'"
                    + " data-rankid='" + rank.getAttribute('rankid') + "'"
                    + " data-count='" + rank.getAttribute('count') + "'"
                    + ">"
                    + "<p class='rank'></p>"
                    + "<a class='ui-btn ui-btn-icon-notext ui-icon-gear' onclick='edit_one_rank(this);'></a>"
                    + "</li>").appendTo(rank_ul);
    var rank_p = rank_li.find("p");
    $("<span class='rank-name'></span>").text(rank.getAttribute('name')).appendTo(rank_p);
    $("<span class='count'></span>").text("(" + rank.getAttribute('count') + ")").appendTo(rank_p);
  }
  rank_ul.listview().listview("refresh");

  rank_ul.sortable({stop: function(event, ui) {
      var data = {action: 'rank.order'};
      $("#ranks_container ul[data-classid=" + classid + "] li").each(function(i) {
        data['rankid_' + (i + 1)] = $(this).attr('data-rankid');
      });
      $.ajax(g_action_url,
             {type: 'POST',
              data: data});
  }});
}

function repopulate_class_list(data) {
  var classes = data.getElementsByTagName("class");
  if (classes.length > 0) {
    $("#ranks_container").empty();
    $("#groups").empty();
    for (var i = 0; i < classes.length; ++i) {
      var cl = classes[i];

      var group_li = $("<li class='ui-li-has-alt'"
                       + " data-classid='" + cl.getAttribute('classid') + "'"
                       + " data-count='" + cl.getAttribute('count') + "'"
                       + " data-nrounds='" + cl.getAttribute('nrounds') + "'"
                       + ">"
                       + "<p></p>"
                       + "<a class='ui-btn ui-btn-icon-notext ui-icon-gear' onclick='edit_one_class(this);'></a>"
                       + "</li>").appendTo("#groups");
      var group_p = group_li.find("p");
      $("<span class='class-name'></span>").text(cl.getAttribute('name')).appendTo(group_p);
      $("<span class='count'></span>").text("(" + cl.getAttribute('count') + ")").appendTo(group_p);

      if (use_subgroups()) {
        populate_rank_list(cl);
      }
    }
  }
}

$(function () {
    $("#groups").sortable({stop: function(event, ui) {
      var data = {action: 'class.order'};
      $("#edit_all_classes_modal ul li").each(function(i) {
        data['classid_' + (i + 1)] = $(this).attr('data-classid');
      });
      
      $.ajax(g_action_url,
             {type: 'POST',
              data: data});
    }});
});
