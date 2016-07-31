$(function () {
    // TODO Disable buttons if there's no current roundid selection.
    $("select").on("change", function(event) {
        // The initial prompt, if present, is shown as a disabled option which
        // we can now remove.
        $(this).find("option:disabled").remove();
        var selection = $(this).find("option:selected");
        $.ajax('action.php',
               {type: 'POST',
                data: {
                   action: 'standings.reveal',
                   roundid: selection.attr('data-roundid')
                }});
      });
});

function handle_reveal1() {
  $.ajax('action.php',
         {type: 'POST',
           data: {
             action: 'standings.reveal',
             expose: '+1'
             }});
}

function handle_reveal_all() {
  $.ajax('action.php',
         {type: 'POST',
           data: {
             action: 'standings.reveal',
             expose: 'all'
             }});
}


function coordinator_poll() {
    $.ajax(g_action_url,
           {type: 'GET',
            data: {query: 'coordinator-poll'},
            success: function(data) {
              setTimeout(coordinator_poll, /* TODO: 2000 */ 6000);
              var kiosks = parse_kiosks(data);
              generate_kiosk_controls(parse_kiosk_pages(data), kiosks);
              $("#kiosk_control_group").trigger("create");
            },
            error: function() {
                setTimeout(coordinator_poll, 2000);
            }
           });
}


$(function() { coordinator_poll(); });
