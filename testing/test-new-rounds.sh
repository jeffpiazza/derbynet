#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

## Testing the "add new round" functionality.  We're just generating the
## rosters, not making schedules or doing any racing.

# We expect there are no rounds beyond a first round for each of the classes
# Note that test-den-changes.sh created and then deleted TheNotLastClass, which
# results in one extra initial round, now deleted
curl_postj action.php "action=json.roster.delete&roundid=8" | check_jfailure

# Top 3 from roundid=1
curl_postj action.php "action=json.roster.new&roundid=1&top=3" | check_jsuccess
jq '.finalists | length' $DEBUG_CURL | expect_eq 3
ROUNDID=`jq '.["new-round"].roundid' $DEBUG_CURL`

# The new round should be roundid=8, which is now deletable
curl_postj action.php "action=json.roster.delete&roundid=$ROUNDID" | check_jsuccess
# roundid=7 is now gone, so second deletion fails
curl_postj action.php "action=json.roster.delete&roundid=$ROUNDID" | check_jfailure

# Top 3 from each rank in roundid=2
curl_postj action.php "action=json.roster.new&roundid=2&top=3&bucketed=1" | check_jsuccess
# TODO: The test data doesn't track subgroups, so there's effectively no
# difference between the bucketed and non-bucketed version.
jq '.finalists | length' $DEBUG_CURL | expect_eq 3

# Grand Finals round, 4 from each den
# In test-basic-checkins, Bears & Freres only have 2 racers, so it's a total of 14 racers
# In test-master-schedule, Bears & Freres have 2 racers, and Webelos only 3, so it's a total of 13 finalists
curl_postj action.php "action=json.roster.new&roundid=&top=4&bucketed=1&roundid_1=1&roundid_2=1&roundid_3=1&roundid_4=1&classname=Grand%20Finals" \
 | check_jsuccess
jq '.finalists | length == 13 or length == 14' $DEBUG_CURL | expect_eq true

ROUNDID=$(jq '.["new-round"].roundid' $DEBUG_CURL)

curl_postj action.php "action=json.roster.delete&roundid=$ROUNDID" | check_jsuccess

# Grand Finals round, top 5 overall
curl_postj action.php "action=json.roster.new&roundid=&top=5&roundid_1=1&roundid_2=1&roundid_3=1&roundid_4=1&classname=Grand%20Finals-2" | check_jsuccess
jq '.finalists | length' $DEBUG_CURL | expect_eq 5

ROUNDID=$(jq '.["new-round"].roundid' $DEBUG_CURL)
curl_postj action.php "action=json.roster.delete&roundid=$ROUNDID" | check_jsuccess

