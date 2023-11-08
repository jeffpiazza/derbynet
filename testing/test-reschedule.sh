#! /bin/bash

BASE_URL=$1

STRESS4LANES=${2:-5}
STRESS6LANES=${3:-$STRESS4LANES}

set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

RESET_SOURCE=reschedule "`dirname $0`/reset-database.sh" "$BASE_URL"
"`dirname $0`/import-roster.sh" "$BASE_URL"


# 4 lanes, 1 masked, 3 in use.
curl_postj action.php "action=settings.write&unused-lane-mask=2&n-lanes=4" | check_jsuccess

# curl_getj "action.php?query=poll&values=racers,rounds"
# curl_getj "action.php?query=racer.list"

# Racer ids for roundid 1 are 1, 6, 11, 16, 21, 26, ..., 81

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

echo ============================= Movement tests ==============================

## Test case: All racers for roundid 1 and 2 checked in, generate schedules,
##   race one (arbitrary) heat in each, then move a raced racer from roundid 1 to roundid 2.
curl_postj action.php "action=schedule.unschedule&roundid=1" | check_jsuccess
curl_postj action.php "action=racer.bulk&what=checkin&who=c1&value=1" | check_jsuccess
curl_postj action.php "action=racer.bulk&what=checkin&who=c2&value=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |  .heats_scheduled == 17" >/dev/null || test_fails
curl_postj action.php "action=schedule.generate&roundid=2" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |  .heats_scheduled == 17" >/dev/null || test_fails

curl_postj action.php "action=heat.select&roundid=1&heat=4" | check_jsuccess
# Racer ids are 41, 51, 66
curl_postj action.php "action=heat.select&now_racing=1" | check_jsuccess
run_heat 1  4  141:1.210 - 151:1.160 166:1.110 x

curl_postj action.php "action=heat.select&roundid=2&heat=6" | check_jsuccess
# Racer ids are 82, 7, 22
curl_postj action.php "action=heat.select&now_racing=1" | check_jsuccess
run_heat 2  6  282:1.234 - 207:1.516 222:1.611 x

curl_getj "action.php?query=poll.coordinator" | \
    jq -e '.["current-heat"] | .roundid == 2 and .heat == 7' \
       >/dev/null || test_fails

echo ============= Move racer 61 from roundid 1 to roundid 2 ======================
curl_postj action.php "action=racer.edit&racerid=61&partitionid=2" | check_jsuccess
# That leaves racer 61 departed from roundid 1 and unscheduled in roundid 2
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .departed == 1 and .unscheduled == 0 and .heats_scheduled == 17" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .departed == 0 and .unscheduled == 1 and .heats_scheduled == 17" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=1" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
            .departed == 0 and .unscheduled == 0 and .heats_scheduled == 16" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .departed == 0 and .unscheduled == 1 and .heats_scheduled == 17" \
       >/dev/null || test_fails

# Having added a new racer to roundid 2, rescheduling adds a new heat.
curl_postj action.php "action=schedule.reschedule&trace=1&roundid=2" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .departed == 0 and .unscheduled == 0 and .heats_scheduled == 16" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
            .departed == 0 and .unscheduled == 0 and .heats_scheduled == 18" \
       >/dev/null || test_fails

echo ============= Move racer 62 from roundid 2 to roundid 1 ======================
curl_postj action.php "action=racer.edit&racerid=62&partitionid=1" | check_jsuccess
# Merely moving the racer doesn't change the schedules
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .departed == 0 and .unscheduled == 1 and .heats_scheduled == 16" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .departed == 1 and .unscheduled == 0 and .heats_scheduled == 18" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=1" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .departed == 0 and .unscheduled == 0 and .heats_scheduled == 17" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
            .departed == 1 and .unscheduled == 0 and .heats_scheduled == 18" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=2" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .departed == 0 and .unscheduled == 0 and .heats_scheduled == 17" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
            .departed == 0 and .unscheduled == 0 and .heats_scheduled == 17" \
       >/dev/null || test_fails

curl_getj "action.php?query=poll.coordinator" | \
    jq -e '.["current-heat"] | .roundid == 2 and .heat == 7 and .now_racing == true' \
       >/dev/null || test_fails

# TODO The last raced heat at this point is heat 5, rather than heat 6,
# so the correct next heat would be heat 6.

echo ============= Move already-raced racer 41 from roundid 1 to roundid 2 ======================
curl_postj action.php "action=racer.edit&racerid=41&partitionid=2" | check_jsuccess
# Merely moving the racer doesn't change the schedules
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .departed == 1 and .unscheduled == 0 and .heats_scheduled == 17" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .departed == 0 and .unscheduled == 1 and .heats_scheduled == 17" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=1" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .departed == 0 and .unscheduled == 0 and .heats_scheduled == 17" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
            .departed == 0 and .unscheduled == 1 and .heats_scheduled == 17" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=2" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .departed == 0 and .unscheduled == 0 and .heats_scheduled == 17" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
            .departed == 0 and .unscheduled == 0 and .heats_scheduled == 18" \
       >/dev/null || test_fails


echo ============= 4 lanes 17 racers roundid 2 ======================

curl_postj action.php "action=database.purge&purge=schedules&roundid=1" | check_jsuccess
curl_postj action.php "action=database.purge&purge=schedules&roundid=2" | check_jsuccess
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
