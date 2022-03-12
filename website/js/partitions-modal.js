'use strict';

// Assumes jquery.js, mobile.js

// partitions is an array of {partitionid, name, count}, in order
function PartitionsModal(div_label, div_label_plural, partitions, callback) {
  var partitions_list;  // The <ul> element holding all the partition <li>'s

  var reorder_modal = 
      $("<div/>").appendTo("body")
      .addClass('modal_dialog hidden block_buttons partition_modal')
      .append($("<form/>")
              .append($("<h3/>").text("Drag to Re-order " + div_label_plural))
              .append($("<div/>")
                      .append(partitions_list = $("<ul/>").addClass('mlistview has-alts')))
              .append($("<input type='button'/>").val("Add " + div_label)
                      .on('click', function() { show_add_partition_modal(); }))
              .append($("<input type='button'/>").val("Close")
                      .on('click', function() { pop_modal(); })));

  var append_li = function(partitionid, name, count) {
    // partitions_list is free
    $("<li/>")
      .appendTo(partitions_list)
      .addClass('mlistview has-alts')
      .attr('data-partitionid', partitionid)
      .attr('data-partition', name)
      .attr('data-count', count)
      .append($("<p/>").text(name)
              .append($('<span/>')
                      .addClass('count')
                      .text(' (' + count + ')')))
      .append($("<a/>").on('click', function() { rename_one_partition(this); }));
  };

  for (var i in partitions) {
    // If a 'Default' partition entry is present, don't allow editing it
    if (partitions[i].partitionid > 0) {
      append_li(partitions[i].partitionid, partitions[i].name, partitions[i].count);
    }
  }
  partitions_list.sortable({
    stop: function(event, ui) {
      var data = {action: 'partition.order'};
      var divids = [];
      partitions_list.find('li').each(function(i) {
        data['partitionid_' + (i + 1)] = $(this).attr('data-partitionid');
        divids.push($(this).attr('data-partitionid'));
      });
      $.ajax('action.php',
             {type: 'POST',
              data: data,
              success: function(res) {
                if (res.outcome.summary == 'success') {
                  callback('reorder', divids);
                }
              }
             });
    }
  });

  var name_field;
  var delete_ext;
  var naming_modal =
      $("<div/>").appendTo("body")
      .addClass('modal_dialog hidden block_buttons partition_naming_modal')
      .append($("<form/>")
              .append($("<h3/>").text(div_label + " Name"))
              .append(name_field = $("<input type='text'/>"))
              .append($("<input type='submit'/>"))
              .append($("<input type='button'/>")
                      .val("Cancel")
                      .on('click', function() { pop_modal(); }))
              .append(delete_ext = $("<div/>")
                      .addClass('delete-extension')
                      .append($("<input type='button'/>")
                              .addClass('delete_button')
                              .val('Delete ' + div_label)
                              .on('click', function() { delete_one_partition(this); })))
             );
  mobile_text(name_field);

  var show_add_partition_modal = function() {
    name_field.val("");
    delete_ext.addClass('hidden');
    push_modal(naming_modal, name_field, function(event) {
      pop_modal();

      $.ajax('action.php',
             {type: 'POST',
              data: {action: 'partition.add',
                     name: name_field.val()},
              success: function(data) {
                if (data.outcome.summary == 'success') {
                  append_li(data.partitionid, name_field.val(), 0);
                  callback('add', {partitionid: data.partitionid,
                                   name: name_field.val()});
                }
              }
             });

      event.preventDefault();
    });
  };

  var rename_one_partition = function(self) {
    var li = $(self).closest('li');
    name_field.val(li.attr('data-partition'));

    delete_ext.toggleClass('hidden', li.attr('data-count') != 0);
    delete_ext.attr('data-partitionid', li.attr('data-partitionid'));

    push_modal(naming_modal, name_field, function(event) {
      pop_modal();
      $.ajax('action.php',
             {type: 'POST',
              data: {action: 'partition.edit',
                     partitionid: li.attr('data-partitionid'),
                     name: name_field.val()},
              success: function(data) {
                if (data.outcome.summary == 'success') {
                  var sp = li.find("p span");
                  li.find("p").text(name_field.val()).append(sp);
                  callback('rename', {partitionid: li.attr('data-partitionid'),
                                      name: name_field.val()});
                }
              }
             });

      event.preventDefault();
    });
  };

  var delete_one_partition = function(self) {
    var del_ext = $(self).parent('[data-partitionid]');
    var divid = del_ext.attr('data-partitionid');
    pop_modal();
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'partition.delete',
                   partitionid: divid},
            success: function(data) {
              if (data.outcome.summary == 'success') {
                var li = partitions_list.find('li[data-partitionid=' + divid + ']');
                li.remove();
                callback('delete', {partitionid: divid});
              }
            }
           });
  };

  return reorder_modal;
}
