console.log("Server started");

const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 8010 });

var by_url = [];

wss.on('connection', function connection(ws, req) {
  if (!by_url.hasOwnProperty(req.url)) {
    by_url[req.url] = [];
  }
  by_url[req.url].push(ws);

  ws.on('error', console.error);

  ws.on('message', function message(message) {
    try {
      var msg_string = message.toString();
      var json = JSON.parse(msg_string);
      console.log((new Date()).toTimeString(), req.url, "from:" + ws.subscriber, json);
      if (json.subscriber != undefined) {
        console.log('   subscription message from ' + json.subscriber);
        ws.subscriber = json.subscriber;
        ws.send(JSON.stringify({type: 'subscription',
                                url: req.url,
                                subscribed: ws.subscriber}));
      } else {
        by_url[req.url].forEach(function each(client) {
          if (client.readyState === WebSocket.OPEN &&
              client != ws &&
              (json.recipient == undefined || json.recipient == client.subscriber)) {
            console.log('  sending to ' + client.subscriber);
            client.send(msg_string);
          }
        });
      }
    } catch (e) {
      console.log('Exception parsing JSON', e);
    }
  });

  ws.on('close', function(reason, description) {
    console.log((new Date()).toTimeString(), req.url, "CLOSING SOCKET:" + ws.subscriber,
                reason, description.toString());
    by_url[req.url] = by_url[req.url].filter(function(el, idx, ar) {
	  return ws != el;
    });
  });

});

// 5s pings to keep all the connections alive
setInterval(function() {
  for (var url in by_url) {
	for (var clienti in by_url[url]) {
	  by_url[url][clienti].ping();
	}
  }
}, 5000);

// Show all the clients every 2min
setInterval(function() {
  console.log('');
  console.log((new Date()).toString());
  console.log('All clients:');
  for (var url in by_url) {
	for (var clienti in by_url[url]) {
	  console.log('   ', url, by_url[url][clienti].subscriber);
	}
  }
  console.log('');
}, 2 * 60 * 1000);
