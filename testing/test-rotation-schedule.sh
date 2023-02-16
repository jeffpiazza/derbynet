#! /bin/bash



BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

"`dirname $0`/reset-database.sh" "$BASE_URL"
"`dirname $0`/import-roster.sh" "$BASE_URL"

curl_postj action.php "action=settings.write&unused-lane-mask=0&n-lanes=6&rotation-schedule=1" | check_jsuccess

# Check in just two cars to start
curl_postj action.php "action=racer.pass&racer=1&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=6&value=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess


TMP=$(mktemp)

jq >$TMP <<EOF
{"schedule": [
[1, 6, null, null, null, null],
[null, 1, 6, null, null, null],
[null, null, 1, 6, null, null],
[null, null, null, 1, 6, null],
[null, null, null, null, 1, 6],
[6, null, null, null, null, 1]
    ]
}
EOF

( curl_getj "action.php?query=poll&values=schedule" | jq | diff $TMP - ) || test_fails

curl_postj action.php "action=schedule.unschedule&roundid=1" | check_jsuccess
# Now some more racers
curl_postj action.php "action=racer.pass&racer=11&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=16&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=21&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=26&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=31&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=36&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=41&value=1" | check_jsuccess
curl_postj action.php "action=racer.pass&racer=46&value=1" | check_jsuccess

curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess

jq >$TMP <<EOF
{"schedule":[
[1, 6, 11, 16, 21, 26],
[46, 1, 6, 11, 16, 21],
[41, 46, 1, 6, 11, 16],
[36, 41, 46, 1, 6, 11],
[31, 36, 41, 46, 1, 6],
[26, 31, 36, 41, 46, 1],
[21, 26, 31, 36, 41, 46],
[16, 21, 26, 31, 36, 41],
[11, 16, 21, 26, 31, 36],
[6, 11, 16, 21, 26, 31]
    ]
}
EOF

curl_getj "action.php?query=poll&values=schedule" | jq | diff $TMP - || test_fails


rm $TMP
