// Requires dashboard-ajax.js

function update_awards_selection() {
  $("li").addClass("hidden");
  $("tr.headings").removeClass("hidden");

  var awardtypeid = $("#awardtype-select").find("option:selected").data('awardtypeid');
  var group = $("#group-select").find("option:selected");
  var classid = group.data('classid');
  var rankid = group.data('rankid');

  var selector = "li";
  if (awardtypeid != null) {
    selector += "[data-awardtypeid='" + awardtypeid + "']";
  }
  if (classid != null) {
    selector += "[data-classid='" + classid + "']";
  }
  if (rankid != null) {
    selector += "[data-rankid='" + rankid + "']";
  }

  $(selector).removeClass("hidden");
}

var g_changing_awards = false;

function on_choose_award(list_item) {
  var item = $(list_item);
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'award.present',
                 key: item.data('awardkey'),
                 reveal: '0'}
         });
  $("#awardname").text(item.data('awardname'));
  $("#recipient").text(item.data('recipient'));
  $("#classname").text(item.data('class'));
  $("#rankname").text(item.data('rank'));
  $(".presenter-inner").removeClass('hidden');
  $("#kiosk-summary").addClass('hidden');
  $("#reveal-checkbox").prop('checked', false);
  g_changing_awards = true;
  try {
    $("#reveal-checkbox").trigger("change", true);
  } finally {
    g_changing_awards = false;
  }
}

function on_reveal() {
  if (!g_changing_awards) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'award.present',
                   reveal: $("#reveal-checkbox").prop('checked') ? 1 : 0}
           });
  }
}

function on_clear_awards() {
  $.ajax(g_action_url,
         {type: 'POST',
          data: {action: 'award.present',
                 key: '',
                 reveal: '0'}
         });
  $("#awardname").text('');
  $("#recipient").text('');
  $("#classname").text('');
  $("#rankname").text('');
  $(".presenter-inner").addClass('hidden');
  $("#kiosk-summary").addClass('hidden');
}

$(function () {
    $("#awardtype-select").on("change", function(event) {
        update_awards_selection();
      });
    $("#group-select").on("change", function(event) {
        update_awards_selection();
      });
  $("input[name='now-presenting'][type='radio']").on("change", function(event) {
    // 'this' will be the now-selected radio element
    AwardRadio.onchange(this); });
});
