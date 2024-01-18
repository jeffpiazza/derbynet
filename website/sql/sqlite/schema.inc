<?php
// SQLite
require_once('inc/schema_version.inc');

if (!function_exists('make_index')) {
  function make_index($table, $column) {
    return "CREATE INDEX IF NOT EXISTS ".$table."_".$column." ON ".$table."(".$column.")";
  }
}

return array_merge(
array(
// "ALTER DATABASE CHARACTER SET utf8",

"DROP TABLE IF EXISTS `Awards`",

"CREATE TABLE `Awards` ("
."  `awardid` INTEGER PRIMARY KEY, "
."  `awardname` VARCHAR(100) NOT NULL, "
."  `awardtypeid` INTEGER NOT NULL DEFAULT 1, "
."  `classid` INTEGER, "
."  `rankid` INTEGER, "
."  `racerid` INTEGER DEFAULT 0, "
."  `sort` INTEGER DEFAULT 0"
.")",
make_index("Awards", "awardtypeid"),
make_index("Awards", "awardname"),
make_index("Awards", "classid"),
make_index("Awards", "rankid"),
make_index("Awards", "sort"),

"DROP TABLE IF EXISTS `AwardTypes`",

"CREATE TABLE `AwardTypes` ("
."  `awardtypeid` INTEGER PRIMARY KEY, "
."  `awardtype` VARCHAR(15) NOT NULL"
.")",
  ),

@include(sql_file_path('audit-table')),

@include(sql_file_path('events-table')),

array(

"DROP TABLE IF EXISTS `Classes`",

"CREATE TABLE `Classes` ("
."  `classid` INTEGER PRIMARY KEY, "
."  `class` VARCHAR(75) NOT NULL UNIQUE COLLATE NOCASE"
.(expected_schema_version() < 3 ? "" :
  // For aggregate classes, constituents is a comma-separated list of classids, and
  // durable is true if the aggregate class was created explicitly by the user, otherwise
  // false if created while creating an aggregate round.
  (",  `constituents` VARCHAR(100) DEFAULT ''"
   .(expected_schema_version() < 5 ? "" :
     ",  `rankids` VARCHAR(100) DEFAULT ''")
   .", `durable` INTEGER"
  // ntrophies is the number of trophies to award for this class, or -1 to use
  // the standard value from settings.
   .", `ntrophies` INTEGER DEFAULT -1"))
.(expected_schema_version() < 2 ? "" : ",  `sortorder` INTEGER")
.")",

"DROP TABLE IF EXISTS ConstituentClasses",

"CREATE TABLE ConstituentClasses ("
."  antecedent INTEGER,"
."  dependent  INTEGER,"
."  UNIQUE (antecedent, dependent)"
.")",

"DROP TABLE IF EXISTS `RaceChart`",

"CREATE TABLE `RaceChart` ("
."  `resultid` INTEGER PRIMARY KEY, "
."  `classid` INTEGER, "
."  `roundid` INTEGER NOT NULL, "
."  `heat` INTEGER NOT NULL, "
."  `lane` INTEGER NOT NULL, "
."  `racerid` INTEGER, "
."  `chartnumber` INTEGER, "
."  `finishtime` DOUBLE NULL, "
."  `finishplace` INTEGER, "
."  `points` INTEGER, "
."  `completed` TIMESTAMP, "
."  `ignoretime` TINYINT(1) DEFAULT 0, "
."  `masterheat` INTEGER DEFAULT 0"
.")",
make_index("RaceChart", "chartnumber"),
make_index("RaceChart", "classid"),
make_index("RaceChart", "finishtime"),
make_index("RaceChart", "heat"),
make_index("RaceChart", "lane"),
make_index("RaceChart", "masterheat"),
make_index("RaceChart", "points"),
make_index("RaceChart", "racerid"),
make_index("RaceChart", "roundid"),

"DROP TABLE IF EXISTS `RaceInfo`",

"CREATE TABLE `RaceInfo` ("
."  `raceinfoid` INTEGER PRIMARY KEY, "
."  `itemkey` VARCHAR(20) NOT NULL, "
.(expected_schema_version() < 2
  ? "  `itemvalue` VARCHAR(50)"
  : "  `itemvalue` VARCHAR(200)")
.")",
make_index("RaceInfo", "itemkey"),

"DROP TABLE IF EXISTS `Ranks`",

"CREATE TABLE `Ranks` ("
."  `rankid` INTEGER PRIMARY KEY, "
."  `rank` VARCHAR(75) NOT NULL COLLATE NOCASE, "
."  `classid` INTEGER NOT NULL"
.(expected_schema_version() < 10 ? "" :
  ", `ntrophies` INTEGER DEFAULT -1")
.(expected_schema_version() < 2 ? "" : ",  `sortorder` INTEGER ")
.")",
make_index("Ranks", "classid"),
make_index("Ranks", "rank"),

"DROP TABLE IF EXISTS `RegistrationInfo`",

"CREATE TABLE `RegistrationInfo` ("
."  `racerid` INTEGER PRIMARY KEY, "
."  `carnumber` INTEGER NOT NULL, "
."  `carname` VARCHAR(30), "
."  `lastname` VARCHAR(30) NOT NULL COLLATE NOCASE, "
."  `firstname` VARCHAR(30) NOT NULL COLLATE NOCASE, "
."  `classid` INTEGER NOT NULL, "
."  `rankid` INTEGER NOT NULL, "
."  `partitionid` INTEGER NOT NULL,"
."  `passedinspection` TINYINT(1) DEFAULT 0, "
."  `imagefile` VARCHAR(255), "
.(expected_schema_version() < 2 ? "" : "  `carphoto` VARCHAR(255), ")
.(expected_schema_version() < REGISTRATION_CHECKIN_SCHEMA ? "" : "  `checkin_time` INTEGER, ")
."  `note` VARCHAR(255), "
."  `exclude` TINYINT(1) DEFAULT 0"
.")",
make_index("RegistrationInfo", "carnumber"),  // Why??
make_index("RegistrationInfo", "classid"),
make_index("RegistrationInfo", "exclude"),
make_index("RegistrationInfo", "lastname"),
make_index("RegistrationInfo", "passedinspection"),
make_index("RegistrationInfo", "rankid"),

"DROP TABLE IF EXISTS `Roster`",

"CREATE TABLE `Roster` ("
."  `rosterid` INTEGER PRIMARY KEY, "
."  `roundid` INTEGER NOT NULL, "
."  `classid` INTEGER NOT NULL DEFAULT 0, "
."  `racerid` INTEGER NOT NULL, "
."  `finalist` TINYINT(1) DEFAULT 0, "
."  `grandfinalist` TINYINT(1) DEFAULT 0"
.")",
make_index("Roster", "classid"),
make_index("Roster", "racerid"),
make_index("Roster", "roundid"),

"DROP TABLE IF EXISTS `Rounds`",

"CREATE TABLE `Rounds` ("
."  `roundid` INTEGER PRIMARY KEY, "
."  `round` INTEGER NOT NULL, "
."  `classid` INTEGER NOT NULL, "
."  `charttype` INTEGER, "
."  `phase` INTEGER"
.")",
make_index("Rounds", "classid"),
make_index("Rounds", "round"),
  ),

@include(sql_file_path('kiosk-table')),

@include(sql_file_path('message-queue-table')),

@include(sql_file_path('scene-tables')),

@include(sql_file_path('playlist-table')),

@include(sql_file_path('award-type-values')),

@include(sql_file_path('balloting')),

@include(sql_file_path('timer-settings')),

@include(sql_file_path('partitions')),

@include(sql_file_path('event-lane-details')),

@include(sql_file_path('action-history')),

array(
"INSERT INTO RaceInfo (itemkey, itemvalue) VALUES ('schema', ".expected_schema_version().")",
"INSERT INTO RaceInfo (itemkey, itemvalue) VALUES ('photos-on-now-racing', 'head')",
"INSERT INTO RaceInfo (itemkey, itemvalue) VALUES ('show-cars-on-deck', '1')",
"INSERT INTO RaceInfo (itemkey, itemvalue) VALUES ('show-racer-photos-rr', '1')",
"INSERT INTO RaceInfo (itemkey, itemvalue) VALUES ('show-car-photos-rr', '1')",
"INSERT INTO RaceInfo (itemkey, itemvalue) VALUES ('upload-videos', '1')",
  )
);
?>
