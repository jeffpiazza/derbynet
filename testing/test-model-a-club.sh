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
    curl_postj action.php \
              "action=racer.import&firstname=Racer-$i&lastname=Racer-$i&classname=Unwashed-Class" | check_jsuccess
done

curl_postj action.php "action=racer.bulk&what=checkin&who=all&value=1" | check_jsuccess

curl_postj action.php "action=racer.bulk&what=number&who=all&start=101" | check_jsuccess

curl_postj action.php "action=settings.write&n-lanes=3&max-runs-per-car=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess

curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
curl_getj "action.php?query=poll.now-racing" | \
    jq '.["current-heat"]["number-of-heats"]' | \
    sed -e 's/"//g' | expect_eq 34
curl_getj "action.php?query=poll.now-racing" | \
    jq '.racers | all((.lane == 1 and .name == "Racer-1 Racer-1") or
                      (.lane == 2 and .name == "Racer-2 Racer-2") or
                      (.lane == 3 and .name == "Racer-3 Racer-3"))' | \
    expect_eq true

curl_postj action.php "action=heat.select&heat=next" | check_jsuccess
curl_getj "action.php?query=poll.now-racing" | \
    jq '.racers | all((.lane == 1 and .name == "Racer-4 Racer-4") or
                      (.lane == 2 and .name == "Racer-5 Racer-5") or
                      (.lane == 3 and .name == "Racer-6 Racer-6"))' | \
    expect_eq true

curl_postj action.php "action=heat.select&heat=next" | check_jsuccess
curl_getj "action.php?query=poll.now-racing" | \
    jq '.racers | all((.lane == 1 and .name == "Racer-7 Racer-7") or
                      (.lane == 2 and .name == "Racer-8 Racer-8") or
                      (.lane == 3 and .name == "Racer-9 Racer-9"))' | \
    expect_eq true

curl_postj action.php "action=heat.select&heat=34" | check_jsuccess
curl_getj "action.php?query=poll.now-racing" | \
    jq '.racers | map(select(.lane == 1 and .name == "Racer-100 Racer-100")) | length' | \
    expect_eq 1
curl_getj "action.php?query=poll.now-racing" | \
    jq '.racers | map(select(.lane == 2 or .lane == 3)) | length' | \
    expect_eq 0
