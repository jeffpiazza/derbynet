#! /bin/bash



BASE_URL=$1

STRESS4LANES=${2:-5}
STRESS6LANES=${3:-$STRESS4LANES}

set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

RESET_SOURCE=reschedule "`dirname $0`/reset-database.sh" "$BASE_URL"
"`dirname $0`/import-roster.sh" "$BASE_URL"


## TODO -
##
## Run one heat of four, with four racers on four lanes.
## Add a late entrant and adjust schedule: five heats.
## Edit the late racer to be in a different group.  Adjust the schedule: five heats with byes.
## Edit the late racer to be back in the group.  Adjust the schedule: seems to be a bug with the last heat.
##
## Add a test for this.

# 4 lanes, all in use
curl_postj action.php "action=settings.write&unused-lane-mask=0&n-lanes=4" | check_jsuccess

# Racer ids for roundid 5 are 5, 10, 15, 20, 25, ..., 80
# Car numbers are 405, 410, ..., 480

# Check in 4 racers (5, 15, 25, 35)
curl_postj action.php "action=racer.pass&racer=5&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=15&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=25&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=35&value=1" | check_jsuccess

curl_postj action.php "action=schedule.generate&roundid=5" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=5&now_racing=1" | check_jsuccess
run_heat 5  1  405:4.050 435:4.350 425:4.250 415:4.150
# 111 101 131 121
# 121 111 101 131
# 131 121 111 101

# Add latecomer 80 (car 480)
curl_postj action.php "action=racer.pass&racer=80&value=1" | check_jsuccess
curl_postj action.php "action=schedule.reschedule&roundid=5&trace=1" | check_jsuccess
jq -e ".chart | .[1] == \"80 5 35 25\" and
                .[2] == \"25 15 80 35\" and
                .[3] == \"35 80 15 5\" and
                .[4] == \"15 25 5 80\"" testing/debug.curl > /dev/null || test_fails

# Oops, it wasn't actually latecomer 80
curl_postj action.php "action=racer.pass&racer=80&value=0" | check_jsuccess
curl_postj action.php "action=schedule.reschedule&roundid=5&trace=1" | check_jsuccess
# So now there are byes in the schedule
jq -e ".chart | .[1] == \"- 5 35 25\" and
                .[2] == \"25 15 - 35\" and
                .[3] == \"35 - 15 5\" and
                .[4] == \"15 25 5 -\"" testing/debug.curl > /dev/null || test_fails

# Oh, latecomer 80 really HAS arrived
curl_postj action.php "action=racer.pass&racer=80&value=1" | check_jsuccess
curl_postj action.php "action=schedule.reschedule&roundid=5&trace=1" | check_jsuccess

# So put them back in the schedule
jq -e ".chart | .[1] == \"80 5 35 25\" and
                .[2] == \"25 15 80 35\" and
                .[3] == \"35 80 15 5\" and
                .[4] == \"15 25 5 80\"" testing/debug.curl > /dev/null || test_fails

RESET_SOURCE=reschedule-2 "`dirname $0`/reset-database.sh" "$BASE_URL"
"`dirname $0`/import-roster.sh" "$BASE_URL"

# 4 lanes, 1 masked, 3 in use.
curl_postj action.php "action=settings.write&unused-lane-mask=2&n-lanes=4" | check_jsuccess

# curl_getj "action.php?query=poll&values=racers,rounds"
# curl_getj "action.php?query=racer.list"

# Racer ids for roundid 1 are 1, 6, 11, 16, 21, 26, ..., 81

# Check in 2 racers for Lions & Tigers (roundid 1)
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
           .heats_scheduled == 17 and (.adjustments |
                  length == 1 and .[0] .why == "'"'"departed"'"'")" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .heats_scheduled == 17 and (.adjustments |
                  length == 1 and .[0] .why == "'"'"unscheduled"'"'")" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=1" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .heats_scheduled == 16 and (.adjustments | length) == 0" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .heats_scheduled == 17 and (.adjustments |
                  length == 1 and .[0] .why == "'"'"unscheduled"'"'")" \
       >/dev/null || test_fails


# Having added a new racer to roundid 2, rescheduling adds a new heat.
curl_postj action.php "action=schedule.reschedule&trace=1&roundid=2" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .heats_scheduled == 16 and (.adjustments | length) == 0" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .heats_scheduled == 18 and (.adjustments | length) == 0" \
       >/dev/null || test_fails

echo ============= Move racer 62 from roundid 2 to roundid 1 ======================
curl_postj action.php "action=racer.edit&racerid=62&partitionid=1" | check_jsuccess
# Merely moving the racer doesn't change the schedules
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .heats_scheduled == 16 and (.adjustments |
                  length == 1 and .[0] .why == "'"'"unscheduled"'"'")" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .heats_scheduled == 18 and (.adjustments |
                  length == 1 and .[0] .why == "'"'"departed"'"'")" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=1" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .heats_scheduled == 17 and (.adjustments | length) == 0" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .heats_scheduled == 18 and (.adjustments |
                  length == 1 and .[0] .why == "'"'"departed"'"'")" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=2" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .heats_scheduled == 17 and (.adjustments | length) == 0" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .heats_scheduled == 17 and (.adjustments | length) == 0" \
       >/dev/null || test_fails

curl_getj "action.php?query=poll.coordinator" | \
    jq -e '.["current-heat"] | .roundid == 2 and .heat == 7 and .now_racing == false' \
       >/dev/null || test_fails

# The last raced heat at this point is heat 5, rather than heat 6,
# so the correct next heat would be heat 6.

echo ============= Move already-raced racer 41 from roundid 1 to roundid 2 ======================
curl_postj action.php "action=racer.edit&racerid=41&partitionid=2" | check_jsuccess
# Merely moving the racer doesn't change the schedules
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .heats_scheduled == 17 and (.adjustments |
                  length == 1 and .[0] .why == "'"'"departed"'"'")" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .heats_scheduled == 17 and (.adjustments |
                  length == 1 and .[0] .why == "'"'"unscheduled"'"'")" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=1" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .heats_scheduled == 17 and (.adjustments | length) == 0" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .heats_scheduled == 17 and (.adjustments |
                  length == 1 and .[0] .why == "'"'"unscheduled"'"'")" \
       >/dev/null || test_fails

curl_postj action.php "action=schedule.reschedule&trace=1&roundid=2" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==1))[0] |
           .heats_scheduled == 17 and (.adjustments | length) == 0" \
       >/dev/null || test_fails
curl_getj "action.php?query=poll.coordinator" | \
    jq -e ".rounds | map(select(.roundid==2))[0] |
           .heats_scheduled == 18 and (.adjustments | length) == 0" \
       >/dev/null || test_fails

curl_postj action.php "action=heat.select&roundid=2&heat=1" | check_jsuccess
curl_postj action.php "action=heat.select&now_racing=1" | check_jsuccess

run_heat 2 1  141:2.685 - 217:1.037  232:1.239
run_heat 2 2  237:2.203 - 161:2.066  141:3.484
run_heat 2 3  242:3.397 - 252:3.462  161:1.462
run_heat 2 4  247:1.374 - 141:1.791  272:3.399 x

curl_postj action.php "action=heat.select&roundid=2&heat=6" | check_jsuccess
curl_postj action.php "action=heat.select&now_racing=1" | check_jsuccess

run_heat 2 6  252:1.033 - 212:2.522  277:3.656
run_heat 2 7  212:1.905 - 222:1.269  237:2.036
run_heat 2 8  217:3.306 - 227:3.274  242:3.259
run_heat 2 9  257:1.455 - 267:2.500  282:1.043
run_heat 2 10 161:3.028 - 272:1.116  202:3.460
run_heat 2 11 222:3.141 - 232:2.654  247:3.970
run_heat 2 12 267:2.015 - 277:2.638  207:1.724
run_heat 2 13 227:3.209 - 237:3.020  252:2.505
run_heat 2 14 232:3.771 - 242:1.665  257:2.728
run_heat 2 15 272:2.702 - 282:3.953  212:2.385
run_heat 2 16 277:1.756 - 202:2.212  217:1.825
run_heat 2 17 202:3.423 - 247:1.878  267:3.701
run_heat 2 18 207:3.607 - 257:1.105  227:2.474 x

curl_postj action.php "action=heat.select&roundid=1&heat=1" | check_jsuccess
curl_postj action.php "action=heat.select&now_racing=1" | check_jsuccess

run_heat 1 1  262:1.220 - 111:3.969 126:2.213
run_heat 1 2  136:1.996 - 262:3.471 131:2.107 x

curl_postj action.php "action=heat.select&roundid=1&heat=4" | check_jsuccess
curl_postj action.php "action=heat.select&now_racing=1" | check_jsuccess

run_heat 1 4  146:1.605 - 156:3.410 171:1.066
run_heat 1 5  181:1.775 - 106:2.603 121:1.963
run_heat 1 6  151:2.006 - 116:2.242 176:3.638
run_heat 1 7  111:2.940 - 121:3.589 136:2.095
run_heat 1 8  116:2.154 - 126:3.967 -
run_heat 1 9  156:3.899 - 166:3.795 181:3.820
run_heat 1 10 106:1.681 - 171:2.746 101:3.013
run_heat 1 11 121:1.838 - 131:2.441 146:1.674
run_heat 1 12 166:2.077 - 176:2.199 106:1.620
run_heat 1 13 126:2.674 - 136:3.644 151:2.212
run_heat 1 14 131:2.028 - -         156:2.573
run_heat 1 15 171:3.340 - 181:1.379 111:2.618
run_heat 1 16 176:1.963 - 101:2.289 262:2.197
run_heat 1 17 101:2.581 - 146:1.692 116:3.143 x

# Racer 41's lone heat in roundid 1 would make it fastest in the round if it
# were counted in standings.  This is to confirm it's not.
curl_getj "action.php?query=standings" | \
    jq -e ".standings | .["'"'"st-r1"'"'"][0][0] == 46 and
                        .["'"'"st-r1"'"'"][1][0] == 51 and
                        .["'"'"st-r1"'"'"][2][0] == 6 and
                        .["'"'"st-r2"'"'"][0][0] == 57 and
                        .["'"'"st-r2"'"'"][1][0] == 22 and
                        .["'"'"st-r2"'"'"][2][0] == 17" \
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
