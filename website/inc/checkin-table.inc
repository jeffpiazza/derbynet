<?php

// These functions construct each row of the main table on the check-in page.

function checkin_flipswitch($racer_id, $first_name, $last_name, $passed) {
    echo '<label for="passed-'.$racer_id.'">Checked In?</label>';
    echo '<input type="checkbox" data-role="flipswitch" name="passed-'.$racer_id.'" id="passed-'.$racer_id.'"';
    if ($passed) echo ' checked="checked"';
    if ($passed && !have_permission(REVERT_CHECK_IN_PERMISSION)) echo ' disabled="disabled"';
    echo ' data-on-text="Yes" data-off-text="No"';
    echo ' onchange=\'handlechange_passed(this, "'.$first_name.' '.$last_name.'");\'/>';
}

function checkin_table_row($racer_id, $first_name, $last_name, $car_no,
                           $rankid, $classname, $rankname, $passed, $exclude, $racer_xbs,
                           $scheduled, $den_scheduled, $pack_xbs, $use_subgroups, $n = 0) {
    echo '<tr class="d'.($n & 1)
          .($den_scheduled ? ' den_scheduled' : '')
          .($exclude ? ' exclude-racer' : '').'">';

    // TODO: Button formatting; data-enhanced="true"
    echo '<td><input type="button" value="Change" onclick=\'show_edit_racer_form('.$racer_id.');\'/></td>';

    echo '<td id="class-'.$racer_id.'" class="sort-class" data-rankid="'.$rankid.'">'.$classname.'</td>';

    if ($use_subgroups)
        echo '<td id="rank-'.$racer_id.'">'.$rankname.'</td>';

    echo '<td class="sort-car-number" id="car-number-'.$racer_id.'">'.$car_no.'</td>';

    echo '<td class="sort-lastname" id="lastname-'.$racer_id.'" data-exclude="'.$exclude.'">'.$last_name.'</td>';
    echo '<td class="sort-firstname" id="firstname-'.$racer_id.'">'.$first_name.'</td>';

    // If a race schedule exists for this racer, don't offer the option to un-pass through this interface.
    // Instead, have go through unscheduling (if that's possible) and then un-pass
    echo '<td class="checkin-status">';
    if ($scheduled) {
        if ($passed) {
            echo 'Racing';
        } else {
            echo 'Scheduled but not passed';
        }
    } else {
        checkin_flipswitch($racer_id, $first_name, $last_name, $passed);
        if ($den_scheduled) echo ' Late!';
    }
    echo '</td>';

    if ($pack_xbs) {
        echo '<td>';
        echo '<label for="xbs-'.$racer_id.'">Exclusively By Scout?</label>';
        echo '<input type="checkbox" data-role="flipswitch" name="xbs-'.$racer_id.'" ';
        if ($racer_xbs) echo ' checked="checked"';
        echo ' data-on-text="Yes" data-off-text="No"';
        echo ' onchange="handlechange_xbs(this);"/>';
        echo '</td>';
    }
    echo '</tr>'."\n";
}
?>