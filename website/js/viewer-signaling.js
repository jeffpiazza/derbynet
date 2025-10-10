'use strict';

function ice_candidate_key(candidate) {
  return "ice-candidate";
}

// viewer_id is string identifying this viewer.
// ideal is the ideal video stream width and height.
// stream_cb gets called when a new stream is added.
function RemoteCamera(viewer_id, ideal, stream_cb) {
  let pc = null;
  pc = new RTCPeerConnection({'iceServers': [{'urls': 'stun:stun.l.google.com:19302'}]});

  let poller = new MessagePoller(
    viewer_id,
    function(msg) {
      if (msg.type == 'offer') {
        logrcvd('offer');
        if (!pc) {
          pc = new RTCPeerConnection({'iceServers': [{'urls': 'stun:stun.l.google.com:19302'}]});
        }
        pc.onnegotiationneeded = function(event) {
          logstream('NEGOTIATION NEEDED');
        };
        pc.onicecandidate = function(event) {
          if (event.candidate) {
            logsent(ice_candidate_key(event.candidate));
            poller.send_message({recipient: 'camera-replay',
                                 type: 'ice-candidate',
                                 from: viewer_id,
                                 candidate: event.candidate.toJSON()});
          }
        };
        pc.ontrack = function(event) {
          let wh = event.track.getSettings();
          logstream('ontrack with ' + wh.width + 'x' + wh.height +
                        ' (' + event.streams.length + ' streams)');
          let s = new MediaStream();
          s.addTrack(event.track);
          stream_cb(s);
        }

        var desc = new RTCSessionDescription(msg.sdp);
        pc.setRemoteDescription(desc)
          .then(function() {
            return pc.createAnswer();
          })
          .then(function(answer) {
            return pc.setLocalDescription(answer);
          })
          .then(function() {
            logsent('answer');
            poller.send_message({recipient: 'camera-replay',
                                 type: 'answer',
                                 from: viewer_id,
                                 sdp: pc.localDescription.toJSON()});
          });
      } else if (msg.type == 'ice-candidate') {
        if (!pc) {
          pc = new RTCPeerConnection(null);
          logstream('UNEXPECTED ICE CANDIDATE');
        }
        let candidate = new RTCIceCandidate(msg.candidate);
        logrcvd(ice_candidate_key(candidate));
        if (msg.from != 'camera-replay') {
          console.log('ICE candidate from unknown sender ' + msg.from + '');
          return;
        }
        pc.addIceCandidate(candidate);
      } else if (msg.type == 'solicitation') {
        // Broadcast solicitation, intended for camera
      } else {
        console.error('Unrecognized message ', msg);
      }
    });

  let pcstate = '';
  let nag = setInterval(function() {
    if (pc) {
      if (pc.connectionState != pcstate) {
        logstream("State " + pc.connectionState);
        pcstate = pc.connectionState;
      }
      if (pc.connectionState == 'disconnected') {
        pc.close();
        pc = null;
      }
    }

    if (pc == null || pc.connectionState != 'connected') {
      logsent("solicitation (nag) " + (pc == null ? 'disconnected' : pc.connectionState) + ' '
              + ideal.width + 'x' + ideal.height);
      poller.send_message({recipient: 'camera-replay', type: 'solicitation',
                           from: viewer_id, ideal: ideal});
    }
  }, 5000);

  // This may fail to send anything if the MessagePoller is using a WebSocket and the WebSocket
  // hasn't finished opening.  ws.onopen is the usual solution
  logsent('solicitation ' + ideal.width + 'x' + ideal.height);
  poller.send_message({recipient: 'camera-replay', type: 'solicitation', from: viewer_id, ideal: ideal});

  this.close = function() {
    logstream('------------ closed -----------------');
    if (poller) {
      poller.close();
    }
    poller = null;
    clearInterval(nag);
  };
}
