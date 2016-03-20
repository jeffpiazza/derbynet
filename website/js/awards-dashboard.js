// Requires dashboard-ajax.js

function present_award(button) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'award.present',
                   key: $(button).attr('data-award')},
            success: function() {
                $(button).parents("tr").css('background-color', '#b3d4fc');
            }
           });
}
