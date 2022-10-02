#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# This script imports 200 racers into a partition called TheTwoHundred and tries
# generating schedules for them.  It doesn't reset the database but it usually
# doesn't need to: it isn't affected by other partitions/classes/rounds.
# Resetting the database before running the test works as well.

# To increase timeouts for a docker container, if necessary, do:
#   docker exec ((container_name)) set-timeout.sh
# or give an explicit timeout value in seconds, e.g.
#   docker exec ((container_name)) set-timeout.sh 900

"`dirname $0`/import-csv-roster.sh" "$BASE_URL" "`dirname $0`/data/extended.csv"

TWO_HUNDRED_CLASS=$(curl_getj "action.php?query=poll.coordinator" \
                        | jq '.rounds | map(select(.class == "TheTwoHundred"))[0].classid')

TWO_HUNDRED_ROUNDID=$(curl_getj "action.php?query=poll.coordinator" \
                        | jq '.rounds | map(select(.class == "TheTwoHundred"))[0].roundid')

RACERID_1050=$(curl_getj "action.php?query=racer.list" | \
                         jq '.racers | map(select( .firstname == "F-1050" ))[0].racerid')
RACERID_1060=$(curl_getj "action.php?query=racer.list" | \
                         jq '.racers | map(select( .firstname == "F-1060" ))[0].racerid')
RACERID_1070=$(curl_getj "action.php?query=racer.list" | \
                         jq '.racers | map(select( .firstname == "F-1070" ))[0].racerid')
RACERID_1080=$(curl_getj "action.php?query=racer.list" | \
                         jq '.racers | map(select( .firstname == "F-1080" ))[0].racerid')
RACERID_1090=$(curl_getj "action.php?query=racer.list" | \
                         jq '.racers | map(select( .firstname == "F-1090" ))[0].racerid')
RACERID_1100=$(curl_getj "action.php?query=racer.list" | \
                         jq '.racers | map(select( .firstname == "F-1100" ))[0].racerid')
RACERID_1110=$(curl_getj "action.php?query=racer.list" | \
                         jq '.racers | map(select( .firstname == "F-1110" ))[0].racerid')
RACERID_1150=$(curl_getj "action.php?query=racer.list" | \
                         jq '.racers | map(select( .firstname == "F-1150" ))[0].racerid')

curl_postj action.php "action=racer.bulk&what=checkin&value=1&who=c$TWO_HUNDRED_CLASS" | check_jsuccess

# 6 lanes, 200 racers, 6 runs each = 1200 heats to schedule
curl_postj action.php "action=settings.write&n-lanes=6&unused-lane-mask=0" | check_jsuccess
SCHED_START=$(date +%s)
# Takes about 15s on my laptop
curl_postj action.php "action=schedule.generate&n_times_per_lane=6&roundid=$TWO_HUNDRED_ROUNDID" > /dev/null
echo $(expr $(date +%s) - $SCHED_START) seconds
# See comment above about increasing timeout for a docker image
check_jsuccess < $DEBUG_CURL

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID '.results | map(select(.roundid == $r)) | length' | \
    expect_eq 7200

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID --argjson rr $RACERID_1100 \
         '.results | map(select(.roundid == $r and .racerid == $rr)) | length' | \
    expect_eq 36

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID --argjson rr $RACERID_1100 \
         '.results | map(select(.roundid == $r and .racerid == $rr and .lane == 1)) | length' | \
    expect_eq 6

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID --argjson rr $RACERID_1100 \
         '.results | map(select(.roundid == $r and .racerid == $rr and .lane == 6)) | length' | \
    expect_eq 6

curl_postj action.php "action=schedule.unschedule&roundid=$TWO_HUNDRED_ROUNDID" | check_jsuccess

# 1 lane, 200 racers, 6 runs each
curl_postj action.php "action=settings.write&n-lanes=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&n_times_per_lane=6&roundid=$TWO_HUNDRED_ROUNDID" | check_jsuccess

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID '.results | map(select(.roundid == $r)) | length' | \
    expect_eq 1200

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID --argjson rr $RACERID_1100 \
         '.results | map(select(.roundid == $r and .racerid == $rr)) | length' | \
    expect_eq 6

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID --argjson rr $RACERID_1100 \
         '.results | map(select(.roundid == $r and .racerid == $rr and .lane == 1)) | length' | \
    expect_eq 6

curl_postj action.php "action=schedule.unschedule&roundid=$TWO_HUNDRED_ROUNDID" | check_jsuccess

# 6 lanes, 3 racers, 6 runs each: 36 heats with 3 racers in each
curl_postj action.php "action=settings.write&n-lanes=6&unused-lane-mask=0" | check_jsuccess
curl_postj action.php "action=racer.bulk&what=checkin&value=0&who=c$TWO_HUNDRED_CLASS" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=$RACERID_1100" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=$RACERID_1050" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=$RACERID_1150" | check_jsuccess
curl_postj action.php "action=schedule.generate&n_times_per_lane=6&roundid=$TWO_HUNDRED_ROUNDID" | check_jsuccess

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID '.results | map(select(.roundid == $r)) | length' | \
    expect_eq 108

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID --argjson rr $RACERID_1100 \
       '.results | map(select(.roundid == $r and .racerid == $rr)) | length' | \
    expect_eq 36

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID --argjson rr $RACERID_1100 \
         '.results | map(select(.roundid == $r and .racerid == $rr and .lane == 1)) | length' | \
    expect_eq 6

curl_postj action.php "action=schedule.unschedule&roundid=$TWO_HUNDRED_ROUNDID" | check_jsuccess

# 6 lanes, 8 racers, 6 runs each: 48 heats
curl_postj action.php "action=racer.pass&racer=$RACERID_1060" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=$RACERID_1070" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=$RACERID_1080" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=$RACERID_1090" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=$RACERID_1110" | check_jsuccess

curl_postj action.php "action=schedule.generate&n_times_per_lane=6&roundid=$TWO_HUNDRED_ROUNDID" | check_jsuccess

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID '.results | map(select(.roundid == $r)) | length' | \
    expect_eq 288

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID --argjson rr $RACERID_1100 \
       '.results | map(select(.roundid == $r and .racerid == $rr)) | length' | \
    expect_eq 36

curl_getj "action.php?query=poll.results&roundid=$TWO_HUNDRED_ROUNDID&details" | \
    jq --argjson r $TWO_HUNDRED_ROUNDID --argjson rr $RACERID_1100 \
         '.results | map(select(.roundid == $r and .racerid == $rr and .lane == 1)) | length' | \
    expect_eq 6

curl_postj action.php "action=schedule.unschedule&roundid=$TWO_HUNDRED_ROUNDID" | check_jsuccess
