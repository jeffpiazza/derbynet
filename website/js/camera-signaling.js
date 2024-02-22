'use strict';

///////////////////////////////////////////////////////////////////////////////////////
//
// Signaling/message handling for the replay camera.
//
// Usage:
//
// Create a single ViewClientManager instance (vcm).
// vcm.setstream( the stream to send out );
//
//////////////////////////////////////////////////////////////////////////////////////
// Assumes message-poller.js

// camera_sent and camera_received assume a logmessage() function defined elsewhere
function camera_sent(viewer, key) {
  logmessage("=> " + viewer + ": " + key);
}
function camera_received(viewer, key) {
  logmessage("<= " + viewer + ": " + key);
}
function ice_candidate_key(candidate) {
  return "ice " + 
    candidate.protocol + " " + candidate.ip + ":" + candidate.port +
    " (" + candidate.type + ")";
}

// A ViewClient represents a remote client that wants to receive the camera stream.
function ViewClient(recipient, poller) {
  let pc = new RTCPeerConnection({'iceServers': [{'urls': 'stun:stun.l.google.com:19302'}]});

  // Local ICE candidate
  pc.onicecandidate = function(event) {
    if (event.candidate) {
      camera_sent(recipient, ice_candidate_key(event.candidate));
      poller.send_message({recipient: recipient,
                           type: 'ice-candidate',
                           from: 'camera-replay',
                           candidate: event.candidate.toJSON()});
    }
  };

  let self = this;
  pc.onnegotiationneeded = function(event) {
    console.log('onnegotiationneeded fires');
    self.convey_offer();
  };

  this.connection = pc;
  
  this.setstream = function(stream) {
    let senders = pc.getSenders();
    for (var k = 0; k < senders.length; ++k) {
      pc.removeTrack(senders[k]);
    }
    stream.getTracks().forEach(function(track) {
      console.log('  adding track');
      pc.addTrack(track, stream);
    });
  };

  this.on_message = function(msg) {
    console.log('on_message ' + msg.type + ' from ' + msg.from);
    if (msg.type == 'answer') {
      this.on_answer(msg);
    } else if (msg.type == 'ice-candidate') {
      this.on_ice_candidate(msg);
    } else if (msg.type == 'subscription') {
      console.log('Subscription acknowledged');
    } else {
      console.error('Unrecognized message for camera:', msg);
      console.trace();
    }
  };
  this.on_answer = function(msg) {
    camera_received(recipient, 'answer');
    pc.setRemoteDescription(new RTCSessionDescription(msg.sdp));
  };
  this.on_ice_candidate = function(msg) {  // Remote ICE candidate signaled
    let candidate = new RTCIceCandidate(msg.candidate);
    camera_received(recipient, ice_candidate_key(candidate));
    pc.addIceCandidate(candidate);
  };

  let ideal = {width: 0, height: 0};
  this.on_solicitation = function(msg) {
    console.log('on_solicitation');
    if (msg.hasOwnProperty('ideal')) {
      ideal = msg.ideal;
    }
    this.convey_offer();
  };
  this.ideal = function() { return ideal; }
  
  this.convey_offer = function() {
    console.log('Conveying offer to ' + recipient);
    pc.createOffer()
      .then(function(offer) {
          return pc.setLocalDescription(offer);
      })
      .then(function() {
        camera_sent(recipient, 'offer');
        poller.send_message({recipient: recipient,
                             type: 'offer',
                             from: 'camera-replay',
                             sdp: pc.localDescription.toJSON()});
      });
  };
}

// The ViewClientManager creates and owns the ViewClient instances and handles
// dispatching messages.
function ViewClientManager(on_add_client_callback) {
  let dispatcher = {};  // viewer-name => ViewClient

  let initializer_cb = function(vc) { };
  
  let poller = new MessagePoller(
    'camera-replay',
    function(msg) {
      if (dispatcher.hasOwnProperty(msg.from)) {
        dispatcher[msg.from].on_message(msg);
      } else if (msg.type != 'solicitation') {
        console.log('Received non-solicitation message from unknown sender: ', msg);
        return;
      } else {
        logmessage("Solicitation received from " + msg.from);
        let client = new ViewClient(msg.from, poller);
        initializer_cb(client);
        dispatcher[msg.from] = client;
        client.on_solicitation(msg);
        on_add_client_callback();
      }
    });

  this.each_client = function(cb) {
    for (var k in dispatcher) {
      cb(dispatcher[k]);
    }
  };

  this.setinitializer = function(cb) {
    initializer_cb = cb;
  };

  this.setstream = function(stream) {
    this.each_client(function (vc) {
      vc.setstream(stream); });
    this.setinitializer(function(vc) {
      vc.setstream(stream);
    });
  };

  // Returns the largest "ideal" resolution requested by a viewer
  this.ideal = function() {
    let ideal = {width: 0, height: 0};
    this.each_client(function(vc) {
      let i = vc.ideal();
      ideal.width = Math.max(ideal.width, i.width);
      ideal.height = Math.max(ideal.height, i.height);
    });
    return ideal;
  };
}
