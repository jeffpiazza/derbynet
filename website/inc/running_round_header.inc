<?php

// This HTML text will also get dynamically updated via update.js,
// so make sure any changes get made in both places!
function running_round_header($now_running, $groups_use_roundid = FALSE) {
  if (@$now_running['class']) {
    $skip_class_name = $now_running['use_master_sched'] || !use_groups();
    echo '<h2 class="now_running">Now racing: '
      .'<a href="#group_'.@$now_running[$groups_use_roundid ? 'roundid' : 'groupid'].'">'
      .($skip_class_name ? ''
        : htmlspecialchars($now_running['class'].', ', ENT_QUOTES, 'UTF-8'))
      .'Round '.@$now_running['round']
      .'</a>'
      .'</h2>'."\n";
  } else {
    echo '<h2 class="now_running">Racing has not yet begun.</h2>'."\n";
  }
}

?>