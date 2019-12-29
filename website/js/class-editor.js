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
  $("#add_class_modal input[name='name']").val("");
  $("#add_class_modal").removeClass("wide_modal");
  $("#aggregate-only").addClass("hidden");
  show_secondary_modal("#add_class_modal", function () {
    close_add_class_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data: $('#add_class_modal form input').not('#aggregate-constituents input').serialize(),
            cache: false,
            headers: { "cache-control": "no-cache" },
            success: function (data) {
              reload_class_list();
            }});

    return false;
  });
}

// Don't allow creating an aggregate class when there are fewer than two
// constituents selected.  This gets attached as an on('changed') function to
// each of the checkboxes for constituents.
function maybe_enable_aggregate_create() {
  $("#add_class_modal input[type='submit']")
    .prop('disabled',
          $("#aggregate-constituents input[type='checkbox']:checked").length < 2);
}

function show_add_aggregate_modal() {
  $("#add_class_modal input[name='name']").val("");
  $("#add_class_modal").addClass("wide_modal");
  $("#aggregate-only").removeClass("hidden");

  $("#aggregate-constituents input[type='checkbox']")
    .prop('checked', false)
    .flipswitch('refresh');

  maybe_enable_aggregate_create();

  show_secondary_modal("#add_class_modal", function () {
    close_add_class_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data: $('#add_class_modal form').serialize(),
            cache: false,
            headers: { "cache-control": "no-cache" },
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
  var ntrophies = list_item.attr('data-ntrophies');
  $("#edit_class_ntrophies option").prop('selected', false);
  if (ntrophies < 0) {
    $("#edit_class_ntrophies option[value='-1']").prop('selected', true)
  } else {
    $('#edit_class_ntrophies option')
      .filter(function () { return $(this).text() == ntrophies; })
      .prop('selected', true);
  }
  $('#edit_class_ntrophies').selectmenu('refresh')

  hide_ranks_except(classid);

  // The "completed rounds" message appears only if there are no native racers,
  // but there are some completed rounds that prevent offering deletion of the
  // class.
  var count = list_item.attr('data-count');
  var nrounds = list_item.attr('data-nrounds');
  $("#completed_rounds_extension").toggleClass('hidden', !(count == 0 && nrounds != 0));
  $("#completed_rounds_count").text(nrounds);

  var constituent_of = list_item.attr('data-constituent-of');
  console.log(list_item.find('.class-name').text() + ': constituent-of ' + constituent_of);
  $("#constituent_extension").toggleClass('hidden', constituent_of == '');
  $("#constituent_owner").text(constituent_of);

  // TODO Don't allow a class deletion if an aggregate class depends on the class.
  $("#delete_class_extension").toggleClass('hidden', !(count == 0 && nrounds == 0 && constituent_of == ''));
  show_edit_one_class_modal(list_item);
}

function show_edit_one_class_modal(list_item) {
    show_secondary_modal("#edit_one_class_modal", function () {
        $.ajax(g_action_url,
               {type: 'POST',
                data: {action: 'class.edit',
                       classid: list_item.attr('data-classid'),
                       name: $("#edit_class_name").val(),
                       ntrophies: $("#edit_class_ntrophies").val()},
                cache: false,
                headers: { "cache-control": "no-cache" },
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
            cache: false,
            headers: { "cache-control": "no-cache" },
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
  $("#add_rank_modal input[name='name']").val("");

  show_tertiary_modal("#add_rank_modal", function() {
    close_add_rank_modal();
    $.ajax(g_action_url,
           {type: 'POST',
            data: $("#add_rank_modal form").serialize(),
            cache: false,
            headers: { "cache-control": "no-cache" },
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
            cache: false,
            headers: { "cache-control": "no-cache" },
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
            cache: false,
            headers: { "cache-control": "no-cache" },
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
            cache: false,
            headers: { "cache-control": "no-cache" },
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
    $("#aggregate-constituents").empty();
    for (var i = 0; i < classes.length; ++i) {
      var cl = classes[i];

      var group_li = $("<li class='ui-li-has-alt'"
                       + " data-classid='" + cl.getAttribute('classid') + "'"
                       + " data-ntrophies='" + cl.getAttribute('ntrophies') + "'"
                       + " data-count='" + cl.getAttribute('count') + "'"
                       + " data-nrounds='" + cl.getAttribute('nrounds') + "'"
                       + " data-constituent-of=''"  // Possibly rewritten below
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
      var constituents = cl.getElementsByTagName('constituent');
      if (constituents.length > 0) {
        var constituents_ul = $('<ul data-role="listview" data-inset="true"></ul>').appendTo(group_li);
        for (var ii = 0; ii < constituents.length; ++ii) {
          $('<li></li>').text(constituents[ii].getAttribute('name')).appendTo(constituents_ul);
        }
      }

      // For the Create Aggregate modal, add a flipswitch for each existing
      // class to the list of potential constituents.
      var flipswitch_div = $('<div class="flipswitch-div"></div>');
      var label = $('<label for="constituent_' + cl.getAttribute('classid') + '"'
                    + ' class="constituent-label"'
                    + '></label>');
      label.text(cl.getAttribute('name'));
      flipswitch_div.append(label);
      flipswitch_div.append(wrap_flipswitch($('<input type="checkbox"'
                                              + ' id="constituent_' + cl.getAttribute('classid') + '"'
                                              + ' name="constituent_' + cl.getAttribute('classid') + '"'
                                              + '/>')));
      $("#aggregate-constituents").append(flipswitch_div);
    }
    $("#aggregate-constituents input[type='checkbox']").on('change', maybe_enable_aggregate_create);
    $("#aggregate-constituents").trigger("create");

    // Mark up constituents so they can't be deleted
    for (var i = 0; i < classes.length; ++i) {
      var cl = classes[i];
      var constituents = cl.getElementsByTagName('constituent');
      for (var ii = 0; ii < constituents.length; ++ii) {
        $("#groups li[data-classid=" + constituents[ii].getAttribute('classid') + "]")
          .attr('data-constituent-of', cl.getAttribute('name'));
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
