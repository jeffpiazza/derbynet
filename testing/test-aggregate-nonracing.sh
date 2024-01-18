#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

RESET_SOURCE=aggregate-nonracing `dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"

curl_postj action.php "action=settings.write&unused-lane-mask=0&n-lanes=4" | check_jsuccess

curl_postj action.php "action=class.edit&classid=1&name=Tiger%20A" | check_jsuccess
# classid 6:
curl_postj action.php "action=class.add&name=Tiger%20B" | check_jsuccess
# classid 7:
curl_postj action.php "action=class.add&constituent_1=1&constituent_6=1&name=Tigers" | check_jsuccess

curl_postj action.php "action=racer.bulk&who=all&what=checkin&value=1" | check_jsuccess

curl_postj action.php "action=racer.edit&rankid=6&racerid=6" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racerid=16" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racerid=26" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racerid=36" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racerid=46" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racerid=56" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racerid=66" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racerid=76" | check_jsuccess

curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=6" | check_jsuccess

curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
run_heat 1 1  101:3.488 121:3.656 141:3.360 181:3.518
run_heat 1 2  111:3.393 131:3.273 151:3.346 101:3.293
run_heat 1 3  121:3.339 141:3.698 161:3.844 111:3.465
run_heat 1 4  131:3.260 151:3.351 171:3.441 121:3.672
run_heat 1 5  141:3.738 161:3.392 181:3.717 131:3.682
run_heat 1 6  151:3.116 171:3.591 101:3.553 141:3.800
run_heat 1 7  161:3.026 181:3.381 111:3.464 151:3.843
run_heat 1 8  171:3.233 101:3.127 121:3.630 161:3.694
run_heat 1 9  181:3.845 111:3.144 131:3.859 171:3.090 x

curl_postj action.php "action=heat.select&roundid=6&now_racing=1" | check_jsuccess
run_heat 6 1  106:3.488 126:3.656 146:3.360 176:3.518
run_heat 6 2  116:3.393 136:3.273 156:3.346 106:3.293
run_heat 6 3  126:3.339 146:3.698 166:3.844 116:3.465
run_heat 6 4  136:3.260 156:3.351 176:3.441 126:3.672
run_heat 6 5  146:3.738 166:3.392 106:3.717 136:3.682
run_heat 6 6  156:3.116 176:3.591 116:3.553 146:3.800
run_heat 6 7  166:3.026 106:3.381 126:2.464 156:2.843
run_heat 6 8  176:3.233 116:3.127 136:3.630 166:3.694  x

curl_postj action.php "action=schedule.generate&roundid=2" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=2&now_racing=1" | check_jsuccess
run_heat 2 1   202:3.711 212:3.108 227:3.420 247:3.463
run_heat 2 2   207:3.397 217:3.019 232:3.792 252:3.081
run_heat 2 3   212:3.710 222:3.874 237:3.474 257:3.757
run_heat 2 4   217:3.140 227:3.717 242:3.816 262:3.197
run_heat 2 5   222:3.104 232:3.395 247:3.402 267:3.868
run_heat 2 6   227:3.788 237:3.473 252:3.666 272:3.803
run_heat 2 7   232:3.325 242:3.485 257:3.645 277:3.643
run_heat 2 8   237:3.878 247:3.302 262:3.033 282:3.393
run_heat 2 9   242:3.751 252:3.123 267:3.489 202:3.344
run_heat 2 10  247:3.178 257:3.648 272:3.843 207:3.805
run_heat 2 11  252:3.647 262:3.093 277:3.248 212:3.845
run_heat 2 12  257:3.085 267:3.576 282:3.474 217:3.451
run_heat 2 13  262:3.563 272:3.702 202:3.107 222:3.841
run_heat 2 14  267:3.488 277:3.266 207:3.650 227:3.687
run_heat 2 15  272:3.245 282:3.477 212:3.846 232:3.571
run_heat 2 16  277:3.291 202:3.811 217:3.786 237:3.189
run_heat 2 17  282:3.642 207:3.182 222:3.571 242:3.311 x

# classid 8
curl_postj action.php "action=class.add&constituent_2=1&constituent_7=1&name=Younger" | check_jsuccess

curl_getj "action.php?query=poll.coordinator" | \
    jq -e '.["ready-aggregate"] | map(.classid) | sort == [7,8]' >/dev/null || test_fails

exit

# roundid 7 for classid 8
curl_postj action.php "action=roster.new&classid=8&top=4&bucketed=1" | check_jsuccess
jq -e '.finalists | map(.racerid) | sort == [1,17,26,47,56,62,71,77]' $DEBUG_CURL >/dev/null || \
    test_fails

curl_postj action.php "action=schedule.generate&roundid=7" | check_jsuccess
