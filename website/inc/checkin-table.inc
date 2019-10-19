<?php

require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');
require_once('inc/awards.inc');  // For classes_and_ranks

// These functions construct each row of the main table on the check-in page.

function checkin_flipswitch($racer_id, $first_name, $last_name, $passed) {
    echo '<label for="passed-'.$racer_id.'">Checked In?</label>';
    echo '<input type="checkbox" data-role="flipswitch" name="passed-'.$racer_id.'" id="passed-'.$racer_id.'"';
    if ($passed) echo ' checked="checked"';
    if ($passed && !have_permission(REVERT_CHECK_IN_PERMISSION)) echo ' disabled="disabled"';
    echo ' data-on-text="Yes" data-off-text="No"';
    echo ' onchange=\'handlechange_passed(this, '
        .htmlspecialchars(json_encode($first_name.' '.$last_name), ENT_QUOTES, 'UTF-8')
        .');\'/>';
}

function checkin_table_row_for_id($racerid) {
  $row = read_single_row(
    'SELECT racerid, carnumber, lastname, firstname, carname, imagefile,'
      .(schema_version() < 2 ? "" : " carphoto,")
    .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, passedinspection, exclude,'
    .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.racerid = RegistrationInfo.racerid) AS scheduled,'
    .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.classid = RegistrationInfo.classid) AS denscheduled,'
    .' EXISTS(SELECT 1 FROM Awards WHERE Awards.awardname = \''.addslashes($xbs_award_name).'\' AND'
    .'                                   Awards.racerid = RegistrationInfo.racerid) AS xbs'
    .' FROM '.inner_join('RegistrationInfo', 'Classes',
                         'RegistrationInfo.classid = Classes.classid',
                         'Ranks',
                         'RegistrationInfo.rankid = Ranks.rankid')
    .' WHERE RegistrationInfo.racerid = :racerid',
    array(':racerid' => $racerid),
    PDO::FETCH_ASSOC);

  list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
  $row['rankseq'] = $ranks[$row['rankid']]['seq'];

  checkin_table_row($row, read_raceinfo_boolean('xbs-award'), read_raceinfo_boolean('use-subgroups'));
}

// $row[]: racerid, firstname, lastname, carnumber, carname, class, rank, rankid, rankseq,
//             passedinspection, imagefile, exclude
//             (rankseq comes from classes_and_ranks' ordering of ranks)
//         xbs - whether this racer has an exclusively-by-scout award
//         scheduled - whether this racer has any RaceChart entries
//         denscheduled - whether any racer in this racer's class have any RaceChart entries
// $pack_xbs: whether exclusively-by-scout awards are shown in this table
// $use_subgroups: whether rank information is relevant
function checkin_table_row(&$row, $pack_xbs, $use_subgroups, $n = 0) {
    echo '<tr class="d'.($n & 1)
          .($row['denscheduled'] ? ' den_scheduled' : '')
          .($row['exclude'] ? ' exclude-racer' : '').'">';

    // TODO: Button formatting; data-enhanced="true", but that's pretty unattractive in this table
    echo '<td><input type="button" value="Change" onclick=\'show_edit_racer_form('.$row['racerid'].');\'/></td>';

    echo '<td id="class-'.$row['racerid'].'"  data-rankid="'.$row['rankid'].'"'
        .' data-rankseq="'.$row['rankseq'].'">'
        .htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8').'</td>';

    if ($use_subgroups)
      echo '<td id="rank-'.$row['racerid'].'">'.htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8').'</td>';

    echo '<td class="sort-car-number" id="car-number-'.$row['racerid'].'">'.$row['carnumber'].'</td>';

    echo '<td id="photo-'.$row['racerid'].'">';

    echo '<a href="javascript:show_racer_photo_modal('.$row['racerid'].')">'
          .'<img class="checkin-photo" data-repo="head" src="'
          .($row[headshots()->column_name()]
          ? headshots()->url_for_racer($row, RENDER_CHECKIN)
          : 'img/photo-headshot.png')
        .'"/>'
        .'</a>';

    // TODO - This causes repeated reads.
    // The issue is presumably null carphoto values
    if (schema_version() >= 2) {
      echo '<a href="javascript:show_car_photo_modal('.$row['racerid'].')">'
          .'<img class="checkin-photo" data-repo="car" src="'
          .($row[car_photo_repository()->column_name()]
            ? car_photo_repository()->url_for_racer($row, RENDER_CHECKIN)
            : 'img/photo-car.png')
          .'"/>'
        .'</a>';
    }

    echo '</td>';

    echo '<td class="sort-lastname" id="lastname-'.$row['racerid'].'" data-exclude="'.$row['exclude'].'">'
        .htmlspecialchars($row['lastname'], ENT_QUOTES, 'UTF-8').'</td>';
    echo '<td class="sort-firstname" id="firstname-'.$row['racerid'].'">'
        .htmlspecialchars($row['firstname'], ENT_QUOTES, 'UTF-8').'</td>';

    echo '<td id="car-name-'.$row['racerid'].'">'
        .htmlspecialchars($row['carname'], ENT_QUOTES, 'UTF-8').'</td>';

    // If a race schedule exists for this racer, don't offer the option to un-pass through this interface.
    // Instead, have go through unscheduling (if that's possible) and then un-pass
    echo '<td class="checkin-status">';
    if ($row['scheduled']) {
        if ($row['passedinspection']) {
            echo 'Racing';
        } else {
            echo 'Scheduled but not passed';
        }
    } else {
        checkin_flipswitch($row['racerid'], $row['firstname'], $row['lastname'], $row['passedinspection']);
        if ($row['denscheduled']) echo ' Late!';
    }
    echo '</td>';

    if ($pack_xbs) {
        echo '<td>';
        echo '<label for="xbs-'.$row['racerid'].'">Exclusively By Scout?</label>';
        echo '<input type="checkbox" data-role="flipswitch" name="xbs-'.$row['racerid'].'" ';
        if ($row['xbs']) echo ' checked="checked"';
        echo ' data-on-text="Yes" data-off-text="No"';
        echo ' onchange="handlechange_xbs(this);"/>';
        echo '</td>';
    }
    echo '</tr>'."\n";
}
?>