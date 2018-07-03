#! /bin/bash


BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"
### Check in every other racer...
`dirname $0`/test-basic-checkins.sh "$BASE_URL"

curl_post action.php "action=settings.write&use-points=1&use-points-checkbox" | check_success
curl_post action.php "action=settings.write&n-lanes=4" | check_success


### Schedule roundid 1
curl_post action.php "action=schedule.generate&roundid=1" | check_success
# TODO Remove me
curl_post action.php "action=schedule.generate&roundid=2" | check_success

# Racing for roundid=1: 5 heats
curl_post action.php "action=select-heat&roundid=1&now_racing=1" | check_success

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success

staged_heat4 101 121 141 111
run_heat_place 1 1   2 3 4 1
staged_heat4 111 131 101 121
run_heat_place 1 2   1 2 3 4
# Report times for this heat, let timer-message compute places
staged_heat4 121 141 111 131
run_heat 1 3   3.3 3.4 3.1 3.2
staged_heat4 131 101 121 141
run_heat_place 1 4   1 2 3 4
staged_heat4 141 111 131 101
run_heat_place 1 5   4 1 2 3  x

# Results (place):
# 111 101 121 141
# 111 131 101 121
# 111 131 121 141
# 131 101 121 141
# 111 131 101 141
#
#  car           first         last      sum(finishplace)  points
# 111 -- DQ                              4                 16
# 131         Felton         Fouche      7                 13
# 101         Adolfo "Dolf"  Asher       10                10
# 121         Derick         Dreier      13                 7
# 141         Jesse          Jara        16                 4

user_login_coordinator
curl_post action.php "action=award.present&key=speed-2-1" | check_success
curl_get "action.php?query=award.current" | expect_one Asher

curl_get "action.php?query=poll.ondeck" | grep 'resultid="1"' | expect_one 'result="2nd"'
curl_get "action.php?query=poll.ondeck" | grep 'resultid="4"' | expect_one 'result="1st"'
