'use strict';


function viewer_sent(key) {
  $("#log").append("<p>=&gt; " + key + "</p>");
}
function viewer_received(key) {
  $("#log").append("<p>&lt;= " + key + "</p>");
}

function make_viewer_id() {
  var alphabet = "abcdefghjklmnpqrstuvwz0123456789";

  var id = "viewer-";
  for (var k = 0; k < 12; ++k) {
    id += alphabet[Math.floor(Math.random() * alphabet.length)];
  }
  return id;
}

function RemoteCamera(viewer_id, stream_cb) {
  let pc = null;

  let poller = new MessagePoller(
    200 /* ms. */, viewer_id,
    function(msg) {
      if (msg.type == 'offer') {
        if (!pc) {
          pc = new RTCPeerConnection(null);
        }
        pc.onicecandidate = function(event) {
          if (event.candidate) {
            viewer_sent('ice-candidate');
            send_message('replay-camera',
                         {type: 'ice-candidate',
                          from: viewer_id,
                          candidate: event.candidate});
          }
        };
        pc.onaddstream = function(event) {  // TODO
          viewer_received('addstream (see it?)');
          stream_cb(event.stream);
        };

        var desc = new RTCSessionDescription(msg.sdp);
        pc.setRemoteDescription(desc)
          .then(function() {
            return pc.createAnswer();
          })
          .then(function(answer) {
            return pc.setLocalDescription(answer);
          })
          .then(function() {
            viewer_sent('answer');
            send_message('replay-camera',
                         {type: 'answer',
                          from: viewer_id,
                          sdp: pc.localDescription});
          });
      } else if (msg.type == 'ice-candidate') {
        if (!pc) {
          pc = new RTCPeerConnection(null);
        }
        viewer_received('ice-candidate');
        if (msg.from != 'replay-camera') {
          console.log('ICE candidate from unknown sender ' + msg.from + '');
          return;
        }
        pc.addIceCandidate(new RTCIceCandidate(msg.candidate));
      } else {
        console.error('Unrecognized message ' + msg);
      }
    });

  let nag = setInterval(function() {
    if (pc) {
      if (pc.connectionState == 'disconnected') {
        pc = null;
      }
    }
    if (!pc) {
      viewer_sent('solicitation (nag)');
      send_message('replay-camera',
                   {type: 'solicitation', from: viewer_id});
    }
  }, 10000);

  viewer_sent('solicitation');
  send_message('replay-camera',
               {type: 'solicitation', from: viewer_id});

  this.stop = function() { nag.clearInterval(); };
}

