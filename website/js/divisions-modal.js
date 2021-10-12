'use strict';

// Assumes jquery.js, mobile.js

// divisions is an array of {divisionid, name, count}, in order
function DivisionsModal(div_label, div_label_plural, divisions, callback) {
  var divisions_list;

  var reorder_modal = 
      $("<div/>").appendTo("body")
      .addClass('modal_dialog hidden block_buttons')
      .append($("<form/>")
              .append($("<h3/>").text("Drag to Re-order " + div_label_plural))
              .append($("<div/>")
                      .append(divisions_list = $("<ul/>").addClass('mlistview has-alts')))
              .append($("<input type='button'/>").val("Add " + div_label)
                      .on('click', function() { show_add_division_modal(); }))
              .append($("<input type='button'/>").val("Close")
                      .on('click', function() { pop_modal(); })));

  var append_li = function(divisionid, name, count) {
    // divisions_list is free
    $("<li/>")
      .appendTo(divisions_list)
      .addClass('mlistview has-alts')
      .attr('data-divisionid', divisionid)
      .attr('data-division', name)
      .attr('data-count', count)
      .append($("<p/>").text(name)
              .append($('<span/>')
                      .addClass('count')
                      .text(' (' + count + ')')))
      .append($("<a/>").on('click', function() { rename_one_division(this); }));
  };

  for (var i in divisions) {
    append_li(divisions[i].divisionid, divisions[i].name, divisions[i].count);
  }
  divisions_list.sortable({
    stop: function(event, ui) {
      var data = {action: 'division.order'};
      var divids = [];
      divisions_list.find('li').each(function(i) {
        data['divisionid_' + (i + 1)] = $(this).attr('data-divisionid');
        divids.push($(this).attr('data-divisionid'));
      });
      $.ajax('action.php',
             {type: 'POST',
              data: data,
              success: function() {
                callback('reorder', divids);
              }
             });
    }
  });

  var name_field;
  var delete_ext;
  var naming_modal =
      $("<div/>").appendTo("body")
      .addClass('modal_dialog hidden block_buttons')
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
                              .on('click', function() { delete_one_division(this); })))
             );
  mobile_text(name_field);

  var show_add_division_modal = function() {
    name_field.val("");
    delete_ext.addClass('hidden');
    show_secondary_modal(naming_modal, name_field, function(event) {
      pop_modal();

      $.ajax('action.php',
             {type: 'POST',
              data: {action: 'division.add',
                     name: name_field.val()},
              success: function(data) {
                append_li(data.divisionid, name_field.val(), 0);
              }
             });

      event.preventDefault();
    });
  };

  var rename_one_division = function(self) {
    var li = $(self).closest('li');
    name_field.val(li.attr('data-division'));

    delete_ext.toggleClass('hidden', li.attr('data-count') != 0);
    delete_ext.attr('data-divisionid', li.attr('data-divisionid'));

    show_secondary_modal(naming_modal, name_field, function(event) {
      pop_modal();
      $.ajax('action.php',
             {type: 'POST',
              data: {action: 'division.edit',
                     divisionid: li.attr('data-divisionid'),
                     name: name_field.val()},
              success: function(data) {
                li.text(name_field.val());
                callback('rename', {divisionid: li.attr('data-divisionid'),
                                    name: name_field.val()});
              }
             });

      event.preventDefault();
    });
  };

  var delete_one_division = function(self) {
    var del_ext = $(self).parent('[data-divisionid]');
    var divid = del_ext.attr('data-divisionid');
    pop_modal();
    $.ajax('action.php',
           {type: 'POST',
            data: {action: 'division.delete',
                   divisionid: divid},
            success: function(data) {
              var li = divisions_list.find('li[data-divisionid=' + divid + ']');
              li.remove();
              callback('delete', {divisionid: divid});
            }
           });
  };

  return reorder_modal;
}
