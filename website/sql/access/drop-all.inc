<?php
// Access
//
// If schema re-initialization is problematic (foreign key constratints?),
// repeatedly execute this script until no more remain.

return array(
'DROP TABLE Awards',
'DROP TABLE AwardTypes',
'DROP TABLE Classes',
'DROP TABLE RaceChart',
'DROP TABLE RaceInfo',
'DROP TABLE Ranks',
'DROP TABLE RegistrationInfo',
'DROP TABLE Roster',
'DROP TABLE Rounds',
);
?>