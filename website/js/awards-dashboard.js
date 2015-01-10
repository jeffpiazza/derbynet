// Requires dashboard-ajax.js

function present_award(button) {
    console.log("Attr key=" + $(button).attr('data-award'));  // TODO
    $.ajax(g_action_url,
           {type: 'GET',  // TODO
            data: {query: 'award-presentations',
                   key: $(button).attr('data-award')},
            success: function() {
                $(button).parents("tr").css('background-color', '#b3d4fc');
            }
           });
}
