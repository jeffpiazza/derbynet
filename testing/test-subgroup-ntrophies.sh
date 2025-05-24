#! /bin/bash


BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

RESET_SOURCE=subgroup-ntrophies \
    `dirname $0`/reset-database.sh "$BASE_URL"

curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
         -X POST -F prefs='@-' -F action=preferences.upload <<EOF | check_jsuccess
group-formation-rule = one-group

GROUP = All Racers
  SUBGROUP = Two
    NTROPHIES = 2
  ENDSUBGROUP
  SIMPLE SUBGROUP = Three
  SUBGROUP = Four
    NTROPHIES = 4
  ENDSUBGROUP
ENDGROUP

n-lanes = 4
use-subgroups
n-rank-trophies = 3
n-den-trophies = 0
n-pack-trophies = 0
finish-formatting = %6.4f
EOF

for SUBG in Two Three Four ; do
    for ABC in A B C D ; do
        curl_postj action.php \
              "action=racer.import&firstname=${ABC}&lastname=${SUBG}-${ABC}&partition=${SUBG}" \
         | check_jsuccess
    done
done

curl_postj action.php "action=racer.bulk&what=checkin&who=all&value=1" | check_jsuccess

curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess

curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess

run_heat 1 1   2.6158 3.7824 2.0463 2.0036
run_heat 1 2   2.1873 2.7640 3.2498 2.7619
run_heat 1 3   3.5283 3.2649 2.3900 2.1653
run_heat 1 4   3.6831 3.9044 3.2217 3.2740
run_heat 1 5   1.010  2.503  1.170  2.508
run_heat 1 6   5.502  2.504  5.518  1.09
run_heat 1 7   2.503  1.050  2.519  5.510
run_heat 1 8   2.504  4.506  3.020  3.011
run_heat 1 9   3.751  3.123  3.489  3.344
run_heat 1 10  3.178  3.648  3.843  3.805
run_heat 1 11  3.647  3.093  3.248  3.349
run_heat 1 12  3.085  3.576  3.474  3.451 x

## Then we expect: 2 speed awards for Two, 3 for Three, 4 for Four

curl_getj "action.php?query=award.list" \
    | jq '.["speed-awards"] | map(select(.rankid==1)) | length' \
    | expect_eq 2

curl_getj "action.php?query=award.list" \
    | jq '.["speed-awards"] | map(select(.rankid==2)) | length' \
    | expect_eq 3

curl_getj "action.php?query=award.list" \
    | jq '.["speed-awards"] | map(select(.rankid==3)) | length' \
    | expect_eq 4


## Re-run the heats with different times, to create a tie for 3rd place in Three

curl_postj action.php "action=database.purge&purge=results" | check_jsuccess

curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess

run_heat 1 1   2.6158 3.7824 2.0463 2.0036
run_heat 1 2   2.1873 2.7640 3.3222 2.7619
run_heat 1 3   3.5283 3.2649 2.3900 2.1653
run_heat 1 4   3.6831 3.9044 3.2217 3.2740
run_heat 1 5   1.010  2.503  1.170  2.508
run_heat 1 6   2.502  2.914  5.518  1.09
run_heat 1 7   2.803  1.050  2.519  2.510
run_heat 1 8   2.8102 4.506  3.020  3.011
run_heat 1 9   3.751  3.123  3.489  3.344
run_heat 1 10  3.178  3.648  3.843  3.805
run_heat 1 11  3.647  3.093  3.248  3.349
run_heat 1 12  3.085  3.576  3.474  3.451 x

# Now there should be 4 speed awards instead of 3 for Three, with two for 3rd
# place.

curl_getj "action.php?query=award.list" \
    | jq '.["speed-awards"] | map(select(.rankid==2)) | length' \
    | expect_eq 4

curl_getj "action.php?query=award.list" \
    | jq '.["speed-awards"] | map(select(.rankid==2 and .place==3)) | length' \
    | expect_eq 2
