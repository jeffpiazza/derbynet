#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"

# Turn on subgroups and split "Lions & Tigers" into 3 subgrouops: "Lions"(1), "Tigers"(6), and "Cougars"(7)
curl_postj action.php "action=settings.write&do-use-subgroups=1&do-use-subgroups-checkbox" | check_jsuccess
curl_postj action.php "action=rank.edit&name=Lions&rankid=1" | check_jsuccess
curl_postj action.php "action=rank.add&classid=1&name=Tigers" | check_jsuccess
curl_postj action.php "action=rank.add&classid=1&name=Cougars" | check_jsuccess

# Move some Lions & Tigers to Tigers (rankid = 6)
curl_postj action.php "action=racer.edit&rankid=6&racer=6" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racer=16" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racer=26" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racer=46" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racer=56" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racer=71" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racer=76" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=6&racer=81" | check_jsuccess

# And 3 "Cougars"
curl_postj action.php "action=racer.edit&rankid=7&racer=21" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=7&racer=36" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=7&racer=41" | check_jsuccess


`dirname $0`/run-scout-heats.sh "$BASE_URL"

# Add a Grand Final by ranks, leaving out rankid=1
RANK_FINAL=`mktemp`
curl_postj action.php "action=roster.new&top=4&bucketed=1&rankid_2=1&rankid_3=1&rankid_4=1&rankid_5=1&rankid_6=1&rankid_7=1&classname=Rank%20Final" | tee $RANK_FINAL | check_jsuccess

# Check that there are 23 in the roster
jq '.finalists | length' $RANK_FINAL | expect_eq 23

# No Lions are included: Adolfo(1), Felton(31), Carroll(11), Levi(51), Owen(61), Raymon(66)
jq -e '.finalists | map(select(.racerid == 1)) | length' $RANK_FINAL | expect_eq 0
jq -e '.finalists | map(select(.racerid == 31)) | length' $RANK_FINAL | expect_eq 0
jq -e '.finalists | map(select(.racerid == 51)) | length' $RANK_FINAL | expect_eq 0
jq -e '.finalists | map(select(.racerid == 61)) | length' $RANK_FINAL | expect_eq 0
jq -e '.finalists | map(select(.racerid == 66)) | length' $RANK_FINAL | expect_eq 0

# Top 4 Tigers: Edgardo(26), Ben(6), Danial(16), Kelvin(46)
jq -e '.finalists | map(select(.racerid == 26)) | length' $RANK_FINAL | expect_eq 1
jq -e '.finalists | map(select(.racerid == 6)) | length' $RANK_FINAL | expect_eq 1
jq -e '.finalists | map(select(.racerid == 16)) | length' $RANK_FINAL | expect_eq 1
jq -e '.finalists | map(select(.racerid == 46)) | length' $RANK_FINAL | expect_eq 1

# Cougars Jesse(41), Herb(36), Derick(21)
jq -e '.finalists | map(select(.racerid == 41)) | length' $RANK_FINAL | expect_eq 1
jq -e '.finalists | map(select(.racerid == 36)) | length' $RANK_FINAL | expect_eq 1
jq -e '.finalists | map(select(.racerid == 21)) | length' $RANK_FINAL | expect_eq 1

curl_postj action.php "action=roster.delete&roundid=6" | check_jsuccess
# Deleting the round (by deleting its roster) seems to leave roundid=6 available
# for the next 'roster.new' operation, below.

## Create "Younger Finals" aggregate of roundid 1,2 and race the round
curl_postj action.php "action=roster.new&top=4&bucketed=1&roundid_1=1&roundid_2=1&classname=Younger%20Finals" | check_jsuccess

curl_postj action.php "action=schedule.generate&roundid=6" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=6&now_racing=1" | check_jsuccess
run_heat	6	1	3.103	3.762	3.359	3.471
run_heat	6	2	3.757	3.635	3.085	3.328
run_heat	6	3	3.586	3.749	3.095	3.494
run_heat	6	4	3.095	3.645	3.836	3.54
run_heat	6	5	3.087	3.278	3.702	3.135
run_heat	6	6	3.602	3.405	3.632	3.523
run_heat	6	7	3.685	3.136	3.737	3.419
run_heat	6	8	3.000	3.477	3.67	3.512 x

## Create "Older Finals" aggregate of roundid 3,4,5, and race
curl_postj action.php "action=roster.new&top=4&bucketed=1&roundid_3=1&roundid_4=1&roundid_5=1&classname=Older%20Finals" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=7" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=7&now_racing=1" | check_jsuccess
run_heat	7	1	3.85	3.145	3.849	3.288
run_heat	7	2	3.706	3.212	3.343	3.711
run_heat	7	3	3.282	3.51	3.703	3.653
run_heat	7	4	3.785	3.835	3.264	3.024
run_heat	7	5	3.081	3.502	3.65	3.256
run_heat	7	6	3.12	3.286	3.209	3.89
run_heat	7	7	3.352	3.673	3.355	3.039
run_heat	7	8	3.11	3.026	3.401	3.152
run_heat	7	9	3.756	3.763	3.38	3.056
run_heat	7	10	3.45	3.692	3.079	3.329
run_heat	7	11	3.003	3.79	3.47	3.199
run_heat	7	12	3.563	3.857	3.255	3.742 x

## Race a second round of Older Finals
curl_postj action.php "action=roster.new&top=8&roundid=7" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=8" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=8&now_racing=1" | check_jsuccess

run_heat	8	1	3.862	3.887	3.162	3.234
run_heat	8	2	3.329	3.401	3.554	3.015
run_heat	8	3	3.674	3.837	3.102	3.5
run_heat	8	4	3.792	3.685	3.096	3.53
run_heat	8	5	3.819	3.501	3.624	3.451
run_heat	8	6	3.183	3.007	3.227	3.665
run_heat	8	7	3.37	3.512	3.448	3.486
run_heat	8	8	3.028	3.104	3.049	3.301 x

## Create a final final of the other two GF's
curl_postj action.php "action=roster.new&top=4&bucketed=1&roundid_8=1&roundid_6=1&classname=Final%20Finals" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=9" | check_jsuccess
curl_postj action.php "action=heat.select&roundid=9&now_racing=1" | check_jsuccess

run_heat	9	1	3.061	3.857	3.565	3.674
run_heat	9	2	3.825	3.672	3.592	3.106
run_heat	9	3	3.392	3.409	3.651	3.577
run_heat	9	4	3.65	3.132	3.415	3.85
run_heat	9	5	3.895	3.052	3.636	3.096
run_heat	9	6	3.598	3.415	3.625	3.831
run_heat	9	7	3.059	3.54	3.306	3.714
run_heat	9	8	3.563	3.657	3.648	3.895 x

