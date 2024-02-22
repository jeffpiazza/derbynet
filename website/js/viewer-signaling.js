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

// viewer_id is string identifying this viewer.
// ideal is the ideal video stream width and height.
// stream_cb gets called when a new stream is added.
function RemoteCamera(viewer_id, ideal, stream_cb) {
  let pc = null;

  let poller = new MessagePoller(
    viewer_id,
    function(msg) {
      if (msg.type == 'offer') {
        if (!pc) {
          pc = new RTCPeerConnection({'iceServers': [{'urls': 'stun:stun.l.google.com:19302'}]});
        }
        pc.onicecandidate = function(event) {
          if (event.candidate) {
            viewer_sent(ice_candidate_key(event.candidate));
            poller.send_message({recipient: 'camera-replay',
                                 type: 'ice-candidate',
                                 from: viewer_id,
                                 candidate: event.candidate.toJSON()});
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
            poller.send_message({recipient: 'camera-replay',
                                 type: 'answer',
                                 from: viewer_id,
                                 sdp: pc.localDescription.toJSON()});
          });
      } else if (msg.type == 'ice-candidate') {
        if (!pc) {
          pc = new RTCPeerConnection(null);
        }
        let candidate = new RTCIceCandidate(msg.candidate);
        viewer_received(ice_candidate_key(candidate));
        if (msg.from != 'camera-replay') {
          console.log('ICE candidate from unknown sender ' + msg.from + '');
          return;
        }
        pc.addIceCandidate(candidate);
      } else if (msg.type == 'solicitation') {
        // Broadcast solicitation, intended for camera
      } else if (msg.type == 'subscription') {
        console.log('Subscription acknowledged');
      } else {
        console.error('Unrecognized message ', msg);
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
      poller.send_message({recipient: 'camera-replay', type: 'solicitation', from: viewer_id, ideal: ideal});
    }
  }, 10000);

  viewer_sent('solicitation');
  poller.send_message({recipient: 'camera-replay', type: 'solicitation', from: viewer_id, ideal: ideal});

  this.close = function() {
    if (poller) {
      poller.close();
    }
    poller = null;
    clearInterval(nag);
  };
}
