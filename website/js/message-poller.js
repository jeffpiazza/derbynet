'use strict';

function MessagePoller(recipient, on_message) {
  if (g_websocket_url != undefined && g_websocket_url != '') {
    const we = new WebSocket(g_websocket_url);

    ws.addEventListener("message", (event) => {
      try {
        var msg = JSON.parse(event.data);
        on_message(msg);
      } catch (e) {
        console.log('  Unable to parse json message:f, event.data');
      }
    });

    ws.addEventListener("open", (event) => {
      ws.send(JSON.stringify({"subscriber": recipient}));
    });

    this.close = function() { if (ws) { ws.close(); ws = null; } };
  } else {
    let poller = this;
    
    this.send_message = function(message_json) {
      $.ajax("action.php",
             {type: 'POST',
              data: {action: 'message.send',
                     message: message_json}});
    };

    this.retrieve_messages = function() {
      $.ajax("action.php",
             {type: 'POST',
              data: {action: 'message.retrieve',
                     recipient: recipient},
              success: function(data) {
                var msgs = data.messages;
                for (var i = 0; i < msgs.length; ++i) {
                  on_message(msgs[i]);
                }
              }
             });
    };

    let interval = setInterval(function() { poller.retrieve_messages(); }, 500 /* ms. */);

    this.close = function() { if (interval) clearInterval(interval); interval = null; };
 }
}
