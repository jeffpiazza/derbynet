'use strict';

function send_message(recipient, message_json) {
  $.ajax("action.php",
         {type: 'POST',
          data: {action: 'message.send',
                 recipient: recipient,
                 message: JSON.stringify(message_json)}});
};

function MessagePoller(ms, recipient, on_message) {
  let interval;

  let poller = this;
  
  this.retrieve_messages = function() {
    $.ajax("action.php",
           {type: 'POST',
            data: {action: 'message.retrieve',
                   recipient: recipient},
            cache: false,
            headers: { "cache-control": "no-cache" },
            success: function(data) {
              // console.log("retrieve_messages took " + ((new Date()).getTime() - start) + "ms.");
              let msgs = data.getElementsByTagName('message');
              for (let i = 0; i < msgs.length; ++i) {
                on_message(JSON.parse(msgs[i].textContent));
              }
            }
           });
  };

  this.set_polling_pace = function(ms) {
    if (interval) {
      clearInterval(interval);
    }
    interval = setInterval(function() { poller.retrieve_messages(); }, ms);
  };

  this.set_polling_pace(ms);
}
