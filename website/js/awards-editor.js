// TODO Ordering in the face of speed trophies?

function awardtypeid_to_awardtype(awardtypeid, awardtypes) {
  for (var i = 0; i < awardtypes.length; ++i) {
    if (awardtypes[i].awardtypeid == awardtypeid) {
      return awardtypes[i].awardtype;
    }
  }
  return "Can't resolve awardtypeid " + awardtypeid;
}

function classid_to_class(classid, classes) {
  if (classid == 0) return "";
  for (var i = 0; i < classes.length; ++i) {
    if (classes[i].classid == classid) {
      return classes[i].name;
    }
  }
  return "Can't resolve classid " + classid;
}

function rankid_to_rank(rankid, classes) {
  if (rankid == 0) return "";
  for (var i = 0; i < classes.length; ++i) {
    for (var j = 0; j < classes[i].subgroups.length; ++j) {
      if (classes[i].subgroups[j].rankid == rankid) {
        return classes[i].subgroups[j].name;
      }
    }
  }
  return "Can't resolve rankid " + rankid;
}

function update_awards(data) {
  var awards = data.awards;

  if ($("#all_awards li").length < awards.length) {
    while ($("#all_awards li").length < awards.length) {
      $('<li></li>')
        .append('<a>' +
                '<span class="awardname"></span>' +
                '<p>' +
                    '<span class="awardtype"></span>' +
                '</p>' +
                '<p class="class-and-rank">' +
                    '<span class="rankname"></span>' +
                    '<span class="classname"></span> ' +
                    '<span class="recipient"></span>' +
                '</p>' +
               '</a>')
        .append('<a onclick="handle_edit_award($(this).closest(\'li\'));"></a>')
        .appendTo('#all_awards');
    }
    // $("#all_awards").listview("refresh");
  }
  
  $("#all_awards li").each(function(i) {
    if (i >= awards.length) {
      $(this).remove();
    } else {
      $(this).attr("data-awardid", awards[i].awardid);
      if ($(this).attr("data-awardname") != awards[i].awardname) {
        $(this).find('span.awardname').text(awards[i].awardname);
        $(this).attr("data-awardname", awards[i].awardname);
      }
      if ($(this).attr("data-awardtypeid") != awards[i].awardtypeid) {
        $(this).attr("data-awardtypeid", awards[i].awardtypeid);
        $(this).find('.awardtype').text(
          awardtypeid_to_awardtype(awards[i].awardtypeid, data['award-types']));
      }

      if ($(this).attr("data-classid") != awards[i].classid) {
        $(this).attr("data-classid", awards[i].classid);
        var classname = classid_to_class(awards[i].classid, data.classes);
        $(this).attr("data-class", classname);
        $(this).find('.classname').text(classname);
      }
      
      if ($(this).attr("data-rankid") != awards[i].rankid) {
        var rankid = awards[i].rankid;
        $(this).attr("data-rankid", rankid);
        var rankname = rankid_to_rank(rankid, data.classes);
        $(this).attr("data-rank", rankname);
        if (rankid != 0) {
          $(this).find('.rankname').text(rankname + ', ');
        }
      }

      if ($(this).attr("data-racerid") != awards[i].racerid) {
        $(this).attr("data-racerid", awards[i].racerid);
        $(this).find('.recipient')
          .empty()
          .text(awards[i].firstname + " " + awards[i].lastname);
      }
    }
  });
}

function handle_new_award() {
  $('#award_editor_form').find('input[name="awardid"]').val('new');
  $('#award_editor_form').find('input[name="name"]').val('');
  // TODO $('#award_editor_form').find('select[name="awardtypeid"]').val(awardtypeid).change();
  // TODO $('#award_editor_form').find('select[name="class_and_rank"]').val('0,0').change();
  $('.delete_button').addClass('hidden');

  show_modal('#award_editor_modal', function() {
    close_modal('#award_editor_modal');
    $.ajax(g_action_url,
           {type: 'POST',
            // award.edit
            data: $('#award_editor_form').serialize(),
            success: function(data) {
              update_awards(data);
            }
           });
    return false;
  });
}

function handle_edit_award(list_item) {
  var awardid = list_item.attr('data-awardid');
  var awardname = list_item.attr('data-awardname');
  var awardtypeid = list_item.attr('data-awardtypeid');
  var rankid = list_item.attr('data-rankid');
  var classid = list_item.attr('data-classid');
  
  $('#award_editor_form').find('input[name="awardid"]').val(awardid);
  $('#award_editor_form').find('input[name="name"]').val(awardname);
  $('#award_editor_form').find('select[name="awardtypeid"]').val(awardtypeid).change();
  $('#award_editor_form').find('select[name="class_and_rank"]').val(classid + ',' + rankid).change();
  $('.delete_button').removeClass('hidden');

  show_modal('#award_editor_modal', function() {
    close_modal('#award_editor_modal');
    $.ajax(g_action_url,
           {type: 'POST',
            // award.edit
            data: $('#award_editor_form').serialize(),
            success: function(data) {
              update_awards(data);
            }
           });
    return false;
  });
}

function handle_delete_award() {
  close_modal('#award_editor_modal');
  if (confirm('Really delete award "' + $('#award_editor_form input[name="name"]').val() + '"?')) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'award.delete',
                   awardid: $('#award_editor_form input[name="awardid"]').val()},
            success: function(data) {
              update_awards(data);
            }
           });
  }
  return false;
}

$(function() {
  $("#all_awards").sortable({stop: function(event, ui) {
    var data = {action: 'award.order'};
    $("#all_awards li").each(function (i) {
      data['awardid_' + (i + 1)] = $(this).attr('data-awardid');
    });

    $.ajax(g_action_url,
           {type: 'POST',
            data: data,
            success: function(data) {
              update_awards(data);
            }
           });
  }});

  // TODO Poll on this, not just this one time
  $.ajax(g_action_url,
         {type: 'GET',
          data: {query: 'award.list',
                 adhoc: '0'},
          success: function(data) {
            update_awards(data);
          }
         });
});

// Design Categories Management
// Map to keep track of current assignments
var designCategoryAssignments = {};
var currentAwardId = null;

function loadDesignCategoryAssignments(awardId) {
  designCategoryAssignments = {};
  currentAwardId = awardId;
  $('#design_category_name').text($('li[data-awardid="' + awardId + '"]').attr('data-awardname'));
  
  // Get the entry count for this award
  updateEntryCount();
  
  // First get all racers
  $.ajax(g_action_url, {
    type: 'GET',
    data: {
      query: 'racer.list'
    },
    success: function(data) {
      var racers = data.racers || [];
      
      // Then get current assignments for this award
      $.ajax(g_action_url, {
        type: 'GET',
        data: {
          query: 'award.eligible-racers',
          awardid: awardId
        },
        success: function(data) {
          var eligibleRacers = data.eligible_racers || [];
          var tbody = $('#racer_assignments tbody');
          tbody.empty();
          
          // Mark eligible racers
          eligibleRacers.forEach(function(racer) {
            designCategoryAssignments[racer.racerid] = true;
          });
          
          // Create table rows for all racers
          racers.forEach(function(racer) {
            if (racer.exclude) return; // Skip excluded racers
            
            var isAssigned = designCategoryAssignments[racer.racerid] || false;
            var row = $('<tr data-racerid="' + racer.racerid + '"></tr>');
            
            row.append('<td>' + racer.carnumber + '</td>');
            row.append('<td>' + racer.firstname + ' ' + racer.lastname + '</td>');
            
            var checkbox = $('<input type="checkbox"' + (isAssigned ? ' checked' : '') + '>');
            checkbox.change(function() {
              toggleDesignCategoryAssignment(awardId, racer.racerid, this.checked);
            });
            
            row.append($('<td></td>').append(checkbox));
            tbody.append(row);
          });
        }
      });
    }
  });
}

function toggleDesignCategoryAssignment(awardId, racerId, assign) {
  $.ajax(g_action_url, {
    type: 'POST',
    data: {
      action: 'design.toggle-entry',
      racerid: racerId,
      awardid: awardId,
      selected: assign ? 'true' : 'false'
    },
    success: function(data) {
      designCategoryAssignments[racerId] = assign;
      updateEntryCount();
    }
  });
}

function updateEntryCount() {
  if (!currentAwardId) return;
  
  $.ajax(g_action_url, {
    type: 'GET',
    data: {
      query: 'design.entry-count',
      awardid: currentAwardId
    },
    success: function(data) {
      var count = data.entry_count || 0;
      $('#entry_count_display').text('Current entries: ' + count + ' racer' + (count != 1 ? 's' : ''));
    }
  });
}

function autoEnterAllRacers() {
  if (!currentAwardId) return;
  
  if (confirm('This will enter ALL racers for this award. Continue?')) {
    $.ajax(g_action_url, {
      type: 'POST',
      data: {
        action: 'design.auto-enter',
        awardid: currentAwardId
      },
      success: function() {
        // Reload assignments
        loadDesignCategoryAssignments(currentAwardId);
        alert('All racers have been entered for this award');
      }
    });
  }
}

// Add buttons to awards for managing design categories
function addDesignCategoryButtons() {
  $('#all_awards li').each(function() {
    var awardtypeid = $(this).attr('data-awardtypeid');
    var awardtype = $(this).find('.awardtype').text();
    
    // Only add button for design awards
    if (awardtype.indexOf('Design') >= 0) {
      if ($(this).find('.design-category-btn').length === 0) {
        var btn = $('<a class="design-category-btn" style="margin-left: 10px; color: blue; cursor: pointer;">Manage Entries</a>');
        btn.click(function(e) {
          e.preventDefault();
          e.stopPropagation();
          var awardId = $(this).closest('li').attr('data-awardid');
          loadDesignCategoryAssignments(awardId);
          show_modal('#design_category_modal');
        });
        
        $(this).find('.class-and-rank').append(btn);
      }
    }
  });
}

// Auto-enter button handler
$(function() {
  $('#auto_enter_all_btn').click(function(e) {
    e.preventDefault();
    autoEnterAllRacers();
  });
  
  // Override the update_awards function to add our buttons
  var originalUpdateAwards = update_awards;
  update_awards = function(data) {
    originalUpdateAwards(data);
    addDesignCategoryButtons();
  };
});
