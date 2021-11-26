var g_timer_last_heard_from = 0;

try {
  const bc = new BroadcastChannel('timer-alive');

  bc.onmessage = function(ev) {
    g_timer_last_heard_from = Date.now();
  }
} catch (e) {
  console.log('timer-alive broadcast channel not available.');
}


function is_timer_alive() {
  return (Date.now() - g_timer_last_heard_from) < 1500;
}

// Either open a new window for timer, or bring an existing one to the fore
function open_timer_window() {
  if (is_timer_alive()) {
    console.log('Timer is already alive');
    var focus = new BroadcastChannel('timer-focus');
    focus.postMessage({focus: true});
    focus.close();
  } else {
    window.open('timer.php', 'timer3',
                'menubar=off,toolbar=off,height=700,width=400,location=off');
  }
  return false;
}
