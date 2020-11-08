<?php

return array(

// For now, and very possibly forever, we recognize only one ballot, so we don't
// bother with a Ballots table, and ballotid is always fixed at 1.
//
// A Ballots entry has a collection of associated BallotAwards entry to define
// what awards are available for that ballot.

"DROP TABLE IF EXISTS `BallotAwards`",

"CREATE TABLE `BallotAwards` ("
." ballotid INTEGER NOT NULL,"
." awardid INTEGER NOT NULL, "
." depth INTEGER NOT NULL"
.")",

"CREATE UNIQUE INDEX IF NOT EXISTS BallotAwards_Primary ON BallotAwards(ballotid, awardid)",
make_index("BallotAwards", "ballotid"),
make_index("BallotAwards", "awardid"),

// Voters are really just identified by string, normally a session identifier.
// when_cast and ip_addr are captured to provide some forensic clues just in
// case something goes wrong.
"DROP TABLE IF EXISTS `Voters`",

"CREATE TABLE `Voters` ("
." voterid INTEGER PRIMARY KEY, "
." ballotid INTEGER NOT NULL, "
." identifier VARCHAR(200), "
." when_cast TIMESTAMP, "
." ip_addr VARCHAR(200)"
.")",
make_index("Voters", "identifier"),

// Ballots and Voters are all in service of capturing Votes!
"DROP TABLE IF EXISTS `Votes`",

"CREATE TABLE `Votes` ("
." voteid INTEGER PRIMARY KEY,"
." awardid INTEGER NOT NULL, "
." racerid INTEGER NOT NULL, "
." voterid INTEGER NOT NULL"
.")",
make_index("Votes", "awardid"),
make_index("Votes", "racerid"),
make_index("Votes", "voterid"),

);

?>