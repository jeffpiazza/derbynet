<?php

return array(

"DROP TABLE BallotAwards",

"CREATE TABLE BallotAwards ("
." ballotid INTEGER NOT NULL,"  // NOT the primary key
." awardid INTEGER NOT NULL, "
." depth INTEGER NOT NULL"
.")",

// "CREATE UNIQUE INDEX BallotAwards_primary ON BallotAwards(ballotid, awardid)",
"CREATE INDEX BallotAwards_ballotid ON BallotAwards(ballotid)",
"CREATE INDEX BallotAwards_awardid ON BallotAwards(awardid)",

// Voters are really just identified by string, normally a session identifier.
// when_cast and ip_addr are captured to provide some forensic clues just in
// case something goes wrong.
"DROP TABLE IF EXISTS Voters",

"CREATE TABLE Voters ("
." voterid COUNTER, "
." ballotid INTEGER, "
." identifier VARCHAR(200), "
." when_cast DATETIME, "
." ip_addr VARCHAR(200)"
.")",
"CREATE UNIQUE INDEX PrimaryKey ON Voters(voterid)",
"CREATE INDEX Voters_identifier ON Voters(identifier)",
, 
"DROP TABLE Votes",

"CREATE TABLE Votes ("
." voteid COUNTER, "
." awardid INTEGER, "
." racerid INTEGER,"
." voterid INTEGER"
.")",
"CREATE UNIQUE INDEX PrimaryKey ON Votes(voteid)",
"CREATE INDEX PKey1 ON Votes(awardid)",
"CREATE INDEX PKey2 ON Votes(racerid)",

);

?>