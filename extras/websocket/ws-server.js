console.log("Server started");

// Websockets connect to ws_port; raw TCP connections (from PHP) come on trigger_port
const ws_port = 8010;
const trigger_port = 8020;

const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 8010 });

var by_url = [];

function forward_msg(url, ws, json, msg_string) {
  if (!by_url.hasOwnProperty(url)) {
    console.log("No websockets from " + url);
    return;
  }
  by_url[url].forEach(function each(client) {
    if (client.readyState === WebSocket.OPEN &&
        client != ws &&
        (json.prefix == undefined || client.subscriber.startsWith(json.prefix)) &&
        (json.recipient == undefined || json.recipient == client.subscriber)) {
      console.log('  forwarding to ' + client.subscriber);
      client.send(msg_string);
    }
  });
}

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
      console.log((new Date()).toUTCString(), req.url, "from:" + ws.subscriber, json);
      if (json.subscriber != undefined) {
        console.log('   subscription message from ' + json.subscriber);
        ws.subscriber = json.subscriber;
        ws.send(JSON.stringify({type: 'subscription',
                                url: req.url,
                                subscribed: ws.subscriber}));
      } else {
        forward_msg(req.url, ws, json, msg_string);
      }
    } catch (e) {
      console.log('Exception parsing JSON', e);
    }
  });

  ws.on('close', function(reason, description) {
    console.log((new Date()).toUTCString(), req.url, "CLOSING SOCKET:" + ws.subscriber,
                reason, description.toString());
    by_url[req.url] = by_url[req.url].filter(function(el, idx, ar) {
	  return ws != el;
    });
  });

});

// 30s pings to keep all the connections alive
setInterval(function() {
  for (var url in by_url) {
	for (var clienti in by_url[url]) {
	  by_url[url][clienti].ping();
	}
  }
}, 30000);

// Show all the clients every 30min
setInterval(function() {
  var first = true;
  for (var url in by_url) {
	for (var clienti in by_url[url]) {
      if (first) {
        console.log('');
        console.log((new Date()).toString());
        console.log('All clients:');
        first = false;
      }
	  console.log('   ', url, by_url[url][clienti].subscriber);
	}
  }
  console.log('');
}, 30 * 60 * 1000);



const net = require("net");

const ch_atsign = 64;
const ch_zero = 48;
const ch_nine = 57;
const ch_newline = 10;

const tcp_server = net.createServer((socket) => {
  console.log("TCP connection accepted");

  var buf = Buffer.alloc(0);
  socket.on("data", (data) => {
    buf = Buffer.concat([buf, data]);

    while (buf.length > 0) {
      if (buf[0] != ch_atsign) {
        console.log("Unrecognized: " + buf.toString());
        socket.end();
      } else {
        var len = 0;
        for (var i = 1; i < buf.length && buf[i] != ch_newline; ++i) {
          if (ch_zero <= buf[i] && buf[i] <= ch_nine) {
            len = len * 10 + (buf[i] - ch_zero);
          } else {
            console.log('Unrecognized at ' + i + ': ' + buffer[i]);
            socket.end();
            break;
          }
        }

        if (i + len < buf.length) {
          try {
            var msg_string = buf.toString('utf8', i + 1, i + 1 + len);
            var json = JSON.parse(msg_string);
            console.log((new Date()).toUTCString(), "trigger:", json);
            forward_msg((new URL(json.url)).pathname, null, json, msg_string);
          } catch(e) {
            console.log('Exception parsing JSON', e);
            socket.end();
            break;
          }
          buf = buf.subarray(i + 1 + len);
        } else {
          console.log('Incomplete message:', i, len, i + len, buf.length);
          break;
        }
      }
    }
  });

  socket.on("end", () => {
    console.log("Client disconnected");
  });

  socket.on("error", (error) => {
    console.log(`Socket Error: ${error.message}`);
  });
});

tcp_server.on("error", (error) => {
  console.log(`Server Error: ${error.message}`);
});

tcp_server.listen(trigger_port, () => {
  console.log(`TCP socket server is running on port: ${trigger_port}`);
});
