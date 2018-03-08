<?php
// MySQL
require_once('inc/schema_version.inc');

return array_merge(
array(
"ALTER DATABASE CHARACTER SET utf8",

"DROP TABLE IF EXISTS `Awards`",

"CREATE TABLE `Awards` ("
."  `AwardID` INTEGER NOT NULL AUTO_INCREMENT, "
."  `AwardName` VARCHAR(50) NOT NULL, "
."  `AwardTypeID` INTEGER NOT NULL DEFAULT 1, "
."  `ClassID` INTEGER, "
."  `RankID` INTEGER, "
."  `RacerID` INTEGER DEFAULT 0, "
."  `Sort` INTEGER DEFAULT 0, "
."  INDEX (`AwardID`), "
."  INDEX (`AwardTypeID`), "
."  INDEX (`AwardName`), "
."  INDEX (`ClassID`), "
."  PRIMARY KEY (`AwardID`), "
."  INDEX (`RankID`), "
."  INDEX (`Sort`)"
.")",

"DROP TABLE IF EXISTS `AwardTypes`",

"CREATE TABLE `AwardTypes` ("
."  `AwardTypeID` INTEGER NOT NULL AUTO_INCREMENT, "
."  `AwardType` VARCHAR(15) NOT NULL, "
."  PRIMARY KEY (`AwardTypeID`)"
.")",
  ),

@include_once(sql_file_path('audit-table')),

array(
"DROP TABLE IF EXISTS `Classes`",

"CREATE TABLE `Classes` ("
."  `ClassID` INTEGER NOT NULL AUTO_INCREMENT, "
."  `Class` VARCHAR(15) NOT NULL, "
.(expected_schema_version() < 2 ? "" : "  `SortOrder` INTEGER, ")
."  UNIQUE (`Class`), "
."  INDEX (`ClassID`), "
."  PRIMARY KEY (`ClassID`)"
.")",

"DROP TABLE IF EXISTS `RaceChart`",

"CREATE TABLE `RaceChart` ("
."  `ResultID` INTEGER NOT NULL AUTO_INCREMENT, "
."  `ClassID` INTEGER NOT NULL, "
."  `RoundID` INTEGER NOT NULL, "
."  `Heat` INTEGER NOT NULL, "
."  `Lane` INTEGER NOT NULL, "
."  `RacerID` INTEGER, "
."  `ChartNumber` INTEGER, "
."  `FinishTime` DOUBLE NULL, "
."  `FinishPlace` INTEGER, "
."  `Points` INTEGER, "
."  `Completed` TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
."  `IgnoreTime` TINYINT(1) DEFAULT 0, "
."  `MasterHeat` INTEGER DEFAULT 0, "
."  INDEX (`ChartNumber`), "
."  INDEX (`ClassID`), "
."  INDEX (`FinishTime`), "
."  INDEX (`Heat`), "
."  INDEX (`Lane`), "
."  INDEX (`MasterHeat`), "
."  INDEX (`Points`), "
."  PRIMARY KEY (`ResultID`), "
."  INDEX (`RacerID`), "
."  INDEX (`RoundID`)"
.")",

"DROP TABLE IF EXISTS `RaceInfo`",

"CREATE TABLE `RaceInfo` ("
."  `RaceInfoID` INTEGER NOT NULL AUTO_INCREMENT, "
."  `ItemKey` VARCHAR(20) NOT NULL, "
.(expected_schema_version() < 2
  ? "  `ItemValue` VARCHAR(50), "
  : "  `ItemValue` VARCHAR(200), ")
."  INDEX (`ItemKey`), "
."  INDEX (`RaceInfoID`), "
."  PRIMARY KEY (`RaceInfoID`)"
.")",

"DROP TABLE IF EXISTS `Ranks`",

"CREATE TABLE `Ranks` ("
."  `RankID` INTEGER NOT NULL AUTO_INCREMENT, "
."  `Rank` VARCHAR(15) NOT NULL, "
."  `ClassID` INTEGER NOT NULL, "
.(expected_schema_version() < 2 ? "" : "  `SortOrder` INTEGER, ")
."  INDEX (`ClassID`), "
."  INDEX (`RankID`), "
."  PRIMARY KEY (`RankID`), "
."  INDEX (`Rank`)"
.")",

"DROP TABLE IF EXISTS `RegistrationInfo`",

"CREATE TABLE `RegistrationInfo` ("
."  `RacerID` INTEGER NOT NULL AUTO_INCREMENT, "
."  `CarNumber` INTEGER NOT NULL, "
."  `CarName` VARCHAR(20), "
."  `LastName` VARCHAR(20) NOT NULL, "
."  `FirstName` VARCHAR(15) NOT NULL, "
."  `ClassID` INTEGER NOT NULL, "
."  `RankID` INTEGER NOT NULL, "
."  `PassedInspection` TINYINT(1) DEFAULT 0, "
."  `ImageFile` VARCHAR(255), "
.(expected_schema_version() < 2 ? "" : "  `CarPhoto` VARCHAR(255), ")
."  `Exclude` TINYINT(1) DEFAULT 0, "
."  INDEX (`CarNumber`), "
."  INDEX (`ClassID`), "
."  INDEX (`Exclude`), "
."  INDEX (`LastName`), "
."  INDEX (`PassedInspection`), "
."  PRIMARY KEY (`RacerID`), "
."  INDEX (`RacerID`), "
."  INDEX (`RankID`)"
.")",

"DROP TABLE IF EXISTS `Roster`",

"CREATE TABLE `Roster` ("
."  `RosterID` INTEGER NOT NULL AUTO_INCREMENT, "
."  `RoundID` INTEGER NOT NULL, "
."  `ClassID` INTEGER NOT NULL DEFAULT 0, "
."  `RacerID` INTEGER NOT NULL, "
."  `Finalist` TINYINT(1) DEFAULT 0, "
."  `GrandFinalist` TINYINT(1) DEFAULT 0, "
."  INDEX (`ClassID`), "
."  PRIMARY KEY (`RosterID`), "
."  INDEX (`RacerID`), "
."  INDEX (`RoundID`), "
."  INDEX (`RosterID`)"
.")",

"DROP TABLE IF EXISTS `Rounds`",

"CREATE TABLE `Rounds` ("
."  `RoundID` INTEGER NOT NULL AUTO_INCREMENT, "
."  `Round` INTEGER NOT NULL, "
."  `ClassID` INTEGER NOT NULL, "
."  `ChartType` INTEGER, "
."  `Phase` INTEGER NOT NULL, "
."  INDEX (`ClassID`), "
."  PRIMARY KEY (`RoundID`), "
."  INDEX (`Round`), "
."  INDEX (`RoundID`)"
.")",
  ),

@include_once(sql_file_path('kiosk-table')),

@include_once(sql_file_path('award-type-values')),

array(
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (10, 'schema', ".expected_schema_version().")",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (11, 'show-racer-photos', '1')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (12, 'show-cars-on-deck', '1')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (13, 'show-racer-photos-rr', '1')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (14, 'show-car-photos-rr', '1')",
  )
);
?>