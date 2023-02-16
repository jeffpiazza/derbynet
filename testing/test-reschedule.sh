#! /bin/bash

BASE_URL=$1

STRESS4LANES=${2:-5}
STRESS6LANES=${3:-$STRESS4LANES}

set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

"`dirname $0`/reset-database.sh" "$BASE_URL"
"`dirname $0`/import-roster.sh" "$BASE_URL"


# 4 lanes, 1 masked, 3 in use.
curl_postj action.php "action=settings.write&unused-lane-mask=2&n-lanes=4" | check_jsuccess

# curl_getj "action.php?query=poll&values=racers,rounds"
# curl_getj "action.php?query=racer.list"

# Racer ids for roundid 1 arg 1, 6, 11, 16, 21, 26, ..., 81

# Checkin 2 racers for Lions & Tigers (roundid 1)
curl_postj action.php "action=racer.pass&racer=1&value=1" | check_jsuccess
# curl_postj action.php "action=racer.pass&racer=6&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=11&value=1" | check_jsuccess

## Test case: 2 racers checked in, 4 lane track with 1 masked lane.  First heat run, add latecomer.
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
# heat 1: 101 -  -  111
# heat 2: 111 - 101  -
# heat 3:  -  - 111 101

run_heat 1  1  101:1.010 - - 111:1.110

curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 2 racers 4 lanes 1 masked 1 heat run add racer 6
jq .chart testing/debug.curl

## Test case: 3 racers checked in, 4 lane track with 1 masked lane.  First heat run, add latecomer.
curl_postj action.php "action=schedule.unschedule&roundid=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=16&value=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
# 101 - 116 111
# 111 - 101 116
# 116 - 111 101
run_heat 1  1  101:1.010 - 116:1.160 111:1.110

curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 3 racers 4 lanes 1 masked 1 heat run add racer 6
jq .chart testing/debug.curl

## Test case: 4 racers checked in, generate schedule, race 1 heat, add latecomer
curl_postj action.php "action=schedule.unschedule&roundid=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=21&value=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
# 101 - 121 116
# 111 - 101 121
# 116 - 111 101
# 121 - 116 111
run_heat 1  1  101:1.010 - 121:1.210 116:1.160
curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 4 racers 4 lanes 1 masked 1 heat run add racer 6
jq .chart testing/debug.curl

run_heat 1  2  111:1.110 - 101:1.010 121:1.210
curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 4 racers 4 lanes 1 masked 2 heats run add racer 6
jq .chart testing/debug.curl

run_heat 1  3  116:1.160 - 111:1.110 101:1.010
curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 4 racers 4 lanes 1 masked 3 heats run add racer 6
jq .chart testing/debug.curl

run_heat 1  4  121:1.210 - 116:1.160 111:1.110 x
curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 4 racers 4 lanes 1 masked 4 heats run add racer 6
jq .chart testing/debug.curl


## Reset
curl_postj action.php "action=schedule.unschedule&roundid=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=21&value=0" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=16&value=0" | check_jsuccess

############## 2 runs per lane
echo ============= 2 runs per lane ======================

curl_postj action.php "action=schedule.generate&n_times_per_lane=2&roundid=1" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
# heat 1: 101 -  -  111
# heat 2: 111 - 101  -
# heat 3:  -  - 111 101

run_heat 1  1  101:1.010 - - 111:1.110

curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 2 racers 4 lanes 1 masked 1 heat run add racer 6
 jq .chart testing/debug.curl

## Test case: 3 racers checked in, 4 lane track with 1 masked lane.  First heat run, add latecomer.
curl_postj action.php "action=schedule.unschedule&roundid=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=16&value=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&n_times_per_lane=2&roundid=1" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
# 101 - 116 111
# 111 - 101 116
# 116 - 111 101
run_heat 1  1  101:1.010 - 116:1.160 111:1.110

curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 3 racers 4 lanes 1 masked 1 heat run add racer 6
jq .chart testing/debug.curl

## Test case: 4 racers checked in, generate schedule, race 1 heat, add latecomer
curl_postj action.php "action=schedule.unschedule&roundid=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=21&value=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&n_times_per_lane=2&roundid=1" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
# 101 - 121 116
# 111 - 101 121
# 116 - 111 101
# 121 - 116 111
run_heat 1  1  101:1.010 - 121:1.210 116:1.160
curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 4 racers 4 lanes 1 masked 1 heat run add racer 6
jq .chart testing/debug.curl

run_heat 1  2  111:1.110 - 101:1.010 121:1.210
curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 4 racers 4 lanes 1 masked 2 heats run add racer 6
jq .chart testing/debug.curl

run_heat 1  3  116:1.160 - 111:1.110 101:1.010
curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 4 racers 4 lanes 1 masked 3 heats run add racer 6
jq .chart testing/debug.curl

run_heat 1  4  121:1.210 - 116:1.160 111:1.110 x
curl_postj action.php "action=schedule.reschedule&roundid=1&racerid=6&dry-run=1&trace=1" | check_jsuccess
echo 4 racers 4 lanes 1 masked 4 heats run add racer 6
jq .chart testing/debug.curl


############## 4 lanes, 17 racers (roundid 2)
echo ============= 4 lanes 17 racers roundid 2 ======================

curl_postj action.php "action=database.purge&purge=schedules&roundid=1" | check_jsuccess
curl_postj action.php "action=settings.write&unused-lane-mask=0&n-lanes=4" | check_jsuccess
curl_postj action.php "action=racer.bulk&what=checkin&who=c2&value=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=2" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=2&now_racing=1" | check_jsuccess

for i in {1..17}
do
    run_heat 2  $i  1.0 2.0 3.0 4.0 x

    echo 17 racers 4 lanes $i heats run add racer 6
    curl_postj action.php "action=schedule.reschedule&roundid=2&racerid=6&dry-run=1" | check_jsuccess
    jq .chart testing/debug.curl
done

# Full coverage would be more like this:
# Loop over number of lanes
#   Loop over lane mask(s)
#     Loop over number of racers checked in for the round (1..nlanes and then high values)
#       Loop over number of heats to run before latecomer
#         Loop over number of latecomers to add
#
# Would take forever to run, though, and would generate a huge number of cases
# to check by hand.

############## Adding racers repeatedly: 4 lanes, 17 racers (roundid 2)
echo ============= Adding racers repeatedly: 4 lanes 17 racers $STRESS4LANES latecomers roundid 2 ======================

curl_postj action.php "action=database.purge&purge=schedules&roundid=2" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=2" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=2&now_racing=1" | check_jsuccess
run_heat 2  1  1.0 2.0 3.0 4.0 x


for i in $( seq 0 $STRESS4LANES )
do
    curl_postj action.php "action=racer.import&firstname=F4-late$i&lastname=L4-late$i&carnumber=90$i&partition=2" | check_jsuccess
    RACERID=$(jq .racerid testing/debug.curl)

    START=$(date +%s)
    curl_postj action.php "action=schedule.reschedule&roundid=2&racerid=$RACERID" | check_jsuccess
    END=$(date +%s)

    jq .chart testing/debug.curl
    jq ".chart | length" testing/debug.curl
    echo "Elapsed time $(($END-$START)) second(s)"
    jq .times testing/debug.curl
done

############## Adding racers repeatedly: 6 lanes, 17 racers (roundid 2)
echo ============= Adding racers repeatedly: 6 lanes 17 racers $STRESS6LANES latecomers roundid 2 ======================

curl_postj action.php "action=database.purge&purge=schedules&roundid=2" | check_jsuccess
curl_postj action.php "action=settings.write&unused-lane-mask=0&n-lanes=6" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=2" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=2&now_racing=1" | check_jsuccess
run_heat 2  1  1.0 2.0 3.0 4.0 5.0 6.0 x


for i in $( seq 0 $STRESS6LANES )
do
    curl_postj action.php "action=racer.import&firstname=F6-late$i&lastname=L6-late$i&carnumber=90$i&partition=2" | check_jsuccess
    RACERID=$(jq .racerid testing/debug.curl)

    START=$(date +%s)
    curl_postj action.php "action=schedule.reschedule&roundid=2&racerid=$RACERID" | check_jsuccess
    END=$(date +%s)

    jq .chart testing/debug.curl
    jq ".chart | length" testing/debug.curl
    echo "Elapsed time $(($END-$START)) second(s)"
    jq .times testing/debug.curl
done

exit
