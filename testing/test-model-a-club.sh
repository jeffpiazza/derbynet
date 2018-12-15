#! /bin/bash

BASE_URL=$1
shift
DBTYPE=$1
shift

set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"


for i in $(seq 1 100) ; do
    curl_post action.php \
              "action=racer.import&firstname=Racer-$i&lastname=Racer-$i&classname=Unwashed-Class&carnumber=$i" | check_success
    curl_post action.php "action=racer.pass&racer=$i&value=1" | check_success
done

curl_post action.php "action=settings.write&n-lanes=3&max-runs-per-car=1" | check_success
curl_post action.php "action=schedule.generate&roundid=1" | check_success

curl_post action.php "action=select-heat&roundid=1&now_racing=1" | check_success
curl_get "action.php?query=poll.now-racing" | expect_one 'number-of-heats="34"'
curl_get "action.php?query=poll.now-racing" | grep 'lane="1"' | expect_one 'Racer-1 Racer-1'
curl_get "action.php?query=poll.now-racing" | grep 'lane="2"' | expect_one 'Racer-2 Racer-2'
curl_get "action.php?query=poll.now-racing" | grep 'lane="3"' | expect_one 'Racer-3 Racer-3'

curl_post action.php "action=select-heat&heat=next" | check_success
curl_get "action.php?query=poll.now-racing" | grep 'lane="1"' | expect_one 'Racer-4 Racer-4'
curl_get "action.php?query=poll.now-racing" | grep 'lane="2"' | expect_one 'Racer-5 Racer-5'
curl_get "action.php?query=poll.now-racing" | grep 'lane="3"' | expect_one 'Racer-6 Racer-6'

curl_post action.php "action=select-heat&heat=next" | check_success
curl_get "action.php?query=poll.now-racing" | grep 'lane="1"' | expect_one 'Racer-7 Racer-7'
curl_get "action.php?query=poll.now-racing" | grep 'lane="2"' | expect_one 'Racer-8 Racer-8'
curl_get "action.php?query=poll.now-racing" | grep 'lane="3"' | expect_one 'Racer-9 Racer-9'

curl_post action.php "action=select-heat&heat=34" | check_success
curl_get "action.php?query=poll.now-racing" | grep 'lane="1"' | expect_one 'Racer-100 Racer-100'
curl_get "action.php?query=poll.now-racing" | expect_count 'lane="2"' 0
curl_get "action.php?query=poll.now-racing" | expect_count 'lane="3"' 0
