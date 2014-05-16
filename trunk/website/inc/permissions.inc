<?php

// Defines individual permissions or privileges
// Permissions are assigned to the session when the user authenticates.
// Permissions can also be assigned for unauthenticated users; see init-roles.php.
// Each permission should be a distinct power of two.

// Controls viewing results organized by racer, rather than by heat.
// Some organizations may wish to disable this if trying to create a
// less competition-oriented environment.
DEFINE('VIEW_RACE_RESULTS_PERMISSION', 1);

// View an awards summary page.  Offering early results may make for a less
// dramatic awards ceremony.
DEFINE('VIEW_AWARDS_PERMISSION', 2);

// For checking in racers, normally for the race crew only:
DEFINE('CHECK_IN_RACERS_PERMISSION', 4);
DEFINE('REVERT_CHECK_IN_PERMISSION', 8);
DEFINE('RENUMBER_CAR_PERMISSION', 16);
DEFINE('REGISTER_NEW_RACER_PERMISSION', 32);
// Allow changing a racer's class (den) or rank (subgroup)
DEFINE('CHANGE_RACER_RANK_PERMISSION', 64);
// Allow changing a rank (subgroup)
DEFINE('ASSIGN_RACER_IMAGE_PERMISSION', 128);
// Assign ad-hoc awards, design awards, Scouts Favorite award
DEFINE('JUDGING_PERMISSION', 256);
// Record heat results, advance heat
DEFINE('CONTROL_RACE_PERMISSION', 512);
// 1024
// 2048
// 4096
// 8192
// 16394
DEFINE('SET_UP_PERMISSION', 32768);
DEFINE('ADMINISTRATION_PERMISSION', 65536);

?>