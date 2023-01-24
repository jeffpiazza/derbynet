<?php
// Access
require_once('inc/schema_version.inc');

return array_merge(
  array(

    // Table constraints prevent deletions from working in alphabetical order.
    'DROP TABLE Awards',
    'DROP TABLE AwardTypes',
    'DROP TABLE RaceInfo',
    'DROP TABLE RaceChart',
    'DROP TABLE Roster',
    'DROP TABLE Rounds',
    'DROP TABLE RegistrationInfo',
    'DROP TABLE Classes',
    'DROP TABLE Ranks',

'DROP TABLE Awards',

'CREATE TABLE Awards'
.'	('
.'	AwardID     COUNTER,'
.'	AwardName   VARCHAR (100),'
.'	AwardTypeID INTEGER,'
.'	ClassID     INTEGER,'
.'	RankID      INTEGER,'
.'	RacerID     INTEGER,'
.'	Sort        INTEGER'
.'	)',

'CREATE UNIQUE INDEX PrimaryKey'
.'	ON Awards (AwardID)',
'CREATE INDEX award_id'
.'	ON Awards (AwardID)',
'CREATE INDEX AwardTypeID'
.'	ON Awards (AwardTypeID)',
'CREATE INDEX AwardTypesAwards'
.'	ON Awards (AwardTypeID)',
'CREATE INDEX ClassID'
.'	ON Awards (AwardName)',
'CREATE INDEX ClassID1'
.'	ON Awards (ClassID)',
'CREATE INDEX RankID'
.'	ON Awards (RankID)',
'CREATE INDEX Sort'
.'	ON Awards (Sort)',

'DROP TABLE AwardTypes',

'CREATE TABLE AwardTypes'
.'	('
.'	AwardTypeID COUNTER,'
.'	AwardType   VARCHAR (15)'
.'	)',

'CREATE UNIQUE INDEX PrimaryKey'
.'	ON AwardTypes (AwardTypeID)',


  ),

@include(sql_file_path('audit-table')),

array(


'DROP TABLE Classes',

'CREATE TABLE Classes'
.'	('
.'	ClassID COUNTER,'
.'	Class   VARCHAR (75)'
.(expected_schema_version() < 2 ? '' : ',	SortOrder INTEGER')
.'	)',

'CREATE UNIQUE INDEX Class'
.'	ON Classes (Class)',

'CREATE UNIQUE INDEX PrimaryKey'
.'	ON Classes (ClassID)',

'CREATE INDEX ID'
.'	ON Classes (ClassID)',



'DROP TABLE ConstituentClasses',

'CREATE TABLE ConstituentClasses'
.'  ('
.'  antecedent INTEGER,'
.'  dependent INTEGER'
.'  )',

'CREATE UNIQUE INDEX AnteDependent'
.'  ON ConstituentClasses (antecedent, dependent)',


'DROP TABLE RaceChart',

'CREATE TABLE RaceChart'
.'	('
.'	ResultID    COUNTER,'
.'	ClassID     INTEGER,'
.'	RoundID     INTEGER,'
.'	Heat        INTEGER,'
.'	Lane        INTEGER,'
.'	RacerID     INTEGER,'
.'	ChartNumber INTEGER,'
.'	FinishTime  DOUBLE,'
.'	FinishPlace INTEGER,'
.'	Points      INTEGER,'
.'	Completed   DATETIME,'
.'	IgnoreTime  BIT,'
.'	MasterHeat  INTEGER'
.'	)',

'CREATE UNIQUE INDEX PrimaryKey'
.'	ON RaceChart (ResultID)',

'CREATE INDEX RaceChart_RacerID'
.'	ON RaceChart (RacerID)',

//'CREATE INDEX RaceChart_ClassID'
//.'	ON RaceChart (ClassID)',

'CREATE INDEX ChartNumber'
.'	ON RaceChart (ChartNumber)',

'CREATE INDEX ClassID'
.'	ON RaceChart (ClassID)',

'CREATE INDEX FinishTime'
.'	ON RaceChart (FinishTime)',

'CREATE INDEX Heat'
.'	ON RaceChart (Heat)',

'CREATE INDEX lane_number'
.'	ON RaceChart (Lane)',

'CREATE INDEX MasterHeat'
.'	ON RaceChart (MasterHeat)',

'CREATE INDEX Points'
.'	ON RaceChart (Points)',

'CREATE INDEX RacerID'
.'	ON RaceChart (RacerID)',

'CREATE INDEX RoundID'
.'	ON RaceChart (RoundID)',

'CREATE INDEX RoundsRaceChart'
.'	ON RaceChart (RoundID)',



'DROP TABLE RaceInfo',

'CREATE TABLE RaceInfo'
.'	('
.'	RaceInfoID COUNTER,'
.'	ItemKey    VARCHAR (20),'
.(expected_schema_version() < 2
  ? '	ItemValue  VARCHAR (50)'
  : '   ItemValue  VARCHAR (200)')
.'	)',

'CREATE UNIQUE INDEX PrimaryKey'
.'	ON RaceInfo (RaceInfoID)',

'CREATE INDEX Code'
.'	ON RaceInfo (ItemKey)',

'CREATE INDEX ID'
.'	ON RaceInfo (RaceInfoID)',



'DROP TABLE Ranks',

'CREATE TABLE Ranks'
.'	('
.'	RankID  COUNTER,'
.'	Rank    VARCHAR (75),'
.'	ClassID INTEGER'
.(expected_schema_version() < 2 ? '' : ',	SortOrder INTEGER')
.'	)',

'CREATE UNIQUE INDEX PrimaryKey'
.'	ON Ranks (RankID)',

//'CREATE INDEX Ranks_ClassID'
//.'	ON Ranks (ClassID)',

'CREATE INDEX ClassID'
.'	ON Ranks (ClassID)',

'CREATE INDEX ID'
.'	ON Ranks (RankID)',

'CREATE INDEX RanksRank'
.'	ON Ranks (Rank)',




'DROP TABLE RegistrationInfo',

'CREATE TABLE RegistrationInfo'
.'	('
.'	RacerID          COUNTER,'
.'	CarNumber        INTEGER,'
.'	CarName          VARCHAR (30),'
.'	LastName         VARCHAR (30),'
.'	FirstName        VARCHAR (30),'
.'	ClassID          INTEGER,'
.'	RankID           INTEGER,'
.'  PartitionID       INTEGER,'
.'	PassedInspection BIT,'
.'	ImageFile        VARCHAR (255),'
.(expected_schema_version() < 2 ? '' : '	CarPhoto        VARCHAR (255),')
.'  Note             VARCHAR (255),'
.'	Exclude          BIT'
.'	)',

'CREATE UNIQUE INDEX PrimaryKey'
.'	ON RegistrationInfo (RacerID)',

//'CREATE INDEX RegistrationInfo_ClassID'
//.'	ON RegistrationInfo (ClassID)',

//'CREATE INDEX RegistrationInfo_RankID'
//.'	ON RegistrationInfo (RankID)',

'CREATE INDEX CarNumber'
.'	ON RegistrationInfo (CarNumber)',

'CREATE INDEX ClassID'
.'	ON RegistrationInfo (ClassID)',

'CREATE INDEX Exclude'
.'	ON RegistrationInfo (Exclude)',

'CREATE INDEX LastName'
.'	ON RegistrationInfo (LastName)',

'CREATE INDEX PassedInspection'
.'	ON RegistrationInfo (PassedInspection)',

'CREATE INDEX RacerID'
.'	ON RegistrationInfo (RacerID)',

'CREATE INDEX RankID'
.'	ON RegistrationInfo (RankID)',


'DROP TABLE Roster',

'CREATE TABLE Roster'
.'	('
.'	RosterID      COUNTER,'
.'	RoundID       INTEGER,'
.'	ClassID       INTEGER,'
.'	RacerID       INTEGER,'
.'	Finalist      BIT,'
.'	GrandFinalist BIT'
.'	)',

'CREATE UNIQUE INDEX PrimaryKey'
.'	ON Roster (RosterID)',

//'CREATE INDEX {1D4487F5-B792-4DE1-BC19-37E72D8F1705}'
//.'	ON Roster (RacerID)',

//'CREATE INDEX {B7991724-2CA8-4272-BD0F-F76C04541457}'
//.'	ON Roster (RoundID)',

'CREATE INDEX ClassID'
.'	ON Roster (ClassID)',

'CREATE INDEX RacerID'
.'	ON Roster (RacerID)',

'CREATE INDEX RoundID'
.'	ON Roster (RoundID)',

'CREATE INDEX ScheduleID'
.'	ON Roster (RosterID)',



'DROP TABLE Rounds',

'CREATE TABLE Rounds'
.'	('
.'	RoundID   COUNTER,'
.'	Round     INTEGER,'
.'	ClassID   INTEGER,'
.'	ChartType INTEGER,'
.'	Phase     INTEGER'
.'	)',

'CREATE UNIQUE INDEX PrimaryKey'
.'	ON Rounds (RoundID)',

//'CREATE INDEX {BAF81037-D72B-466C-BCA9-8C845F7259E7}'
//.'	ON Rounds (ClassID)',

'CREATE INDEX ClassID'
.'	ON Rounds (ClassID)',

'CREATE INDEX Round'
.'	ON Rounds (Round)',

'CREATE INDEX RoundID'
.'	ON Rounds (RoundID)',
  ),

@include(sql_file_path('kiosk-table')),

@include(sql_file_path('events-table')),

@include(sql_file_path('message-queue-table')),

@include(sql_file_path('scene-tables')),

@include(sql_file_path('playlist-table')),

@include(sql_file_path('award-type-values')),

@include(sql_file_path('balloting')),

@include(sql_file_path('timer-settings')),

@include(sql_file_path('partitions')),

@include(sql_file_path('event-lane-details')),

array(

// --- Populate basic data

"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (1, 'H1', 'Title1')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (2, 'H2', 'Title2')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (4, 'ClassID', '7')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (5, 'RoundID', '7')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (6, 'Heat', '1')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (12, 'StepDataFile', 'Complete')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (13, 'StepSWSetup', 'Incomplete')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (14, 'StepReportsSetup', 'Incomplete')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (15, 'StepHWSetup', 'Complete')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (16, 'StepDefineAwards', 'Incomplete')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (17, 'StepRegistration', 'Incomplete')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (18, 'StepSchedules', 'Incomplete')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (19, 'StepRacing', 'Incomplete')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (20, 'StepAwardsCeremony', 'Incomplete')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (21, 'StepOrgSetup', 'Complete')",

"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (10, 'schema', ".expected_schema_version().")",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (11, 'photos-on-now-racing', 'head')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (12, 'show-cars-on-deck', '1')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (13, 'show-racer-photos-rr', '1')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (14, 'show-car-photos-rr', '1')",
"INSERT INTO RaceInfo (RaceInfoID, ItemKey, ItemValue) VALUES (15, 'upload-videos', '1')",
  )
);
?>
