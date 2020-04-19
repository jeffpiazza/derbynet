'use strict';


function viewer_sent(key) {
  logmessage("=> " + key);
}
function viewer_received(key) {
  logmessage("<= " + key);
}
function ice_candidate_key(candidate) {
  return "ice " + 
    candidate.protocol + " " + candidate.address + ":" + candidate.port +
    " (" + candidate.type + ")";
}


function make_viewer_id() {
  var alphabet = "abcdefghjklmnpqrstuvwz0123456789";

  var id = "viewer-";
  for (var k = 0; k < 12; ++k) {
    id += alphabet[Math.floor(Math.random() * alphabet.length)];
  }
  return id;
}

// viewer_id is string identifying this viewer.
// ideal is the ideal video stream width and height.
// stream_cb gets called when a new stream is added.
function RemoteCamera(viewer_id, ideal, stream_cb) {
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
            viewer_sent(ice_candidate_key(event.candidate));
            send_message('replay-camera',
                         {type: 'ice-candidate',
                          from: viewer_id,
                          candidate: event.candidate});
          }
        };
        // TODO onaddstream is obsolete; ontrack should be used instead.
        // The track event is specified to include an optional streams member,
        // but its only optional.
        pc.onaddstream = function(event) {
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
        let candidate = new RTCIceCandidate(msg.candidate);
        viewer_received(ice_candidate_key(candidate));
        if (msg.from != 'replay-camera') {
          console.log('ICE candidate from unknown sender ' + msg.from + '');
          return;
        }
        pc.addIceCandidate(candidate);
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
                   {type: 'solicitation', from: viewer_id, ideal: ideal});
    }
  }, 10000);

  viewer_sent('solicitation');
  send_message('replay-camera',
               {type: 'solicitation', from: viewer_id, ideal: ideal});

  this.stop = function() { nag.clearInterval(); };
}

