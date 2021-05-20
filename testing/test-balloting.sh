#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator


`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"
curl_post action.php "action=racer.bulk&what=checkin&who=all" | check_success

curl_postj action.php "action=json.award.import&awardname=Best%20in%20Show&awardtype=Design%20Trophy" | check_jsuccess
curl_postj action.php "action=json.award.import&awardname=Consolation%20Prize&awardtype=Design%20Trophy" | check_jsuccess
curl_postj action.php "action=json.award.import&awardname=Sore%20Loser&awardtype=Design%20Trophy" | check_jsuccess

curl_postj action.php "action=json.award.import&awardname=Best%20Wolf&classname=White's%20Wolves&awardtype=Design%20Trophy" | check_jsuccess


# awardid 1 : Best in Show
# awardid 2 : Consolation Prize
# awardid 3 : Sore Loser
# awardid 4 : Best Wolf

curl_postj action.php "action=json.award.edit&awardid=1&sort=1&ballot_depth=3" | check_jsuccess
curl_postj action.php "action=json.award.edit&awardid=2&ballot_depth=3" | check_jsuccess
curl_postj action.php "action=json.award.edit&awardid=2&ballot_depth=0" | check_jsuccess
curl_postj action.php "action=json.award.edit&awardid=3&sort=2&ballot_depth=2" | check_jsuccess
curl_postj action.php "action=json.award.edit&awardid=4&sort=3&ballot_depth=1" | check_jsuccess

# Voting not yet open
curl_post action.php "action=vote.cast&awardid=3&votes=[12,36]" | check_failure

curl_postj action.php "action=json.ballot.open&state=open" | check_jsuccess

curl_post action.php "action=vote.cast&awardid=3&votes=[16,42]" | check_success
# The same voter selecting different candidates doesn't add to the original choices
curl_post action.php "action=vote.cast&awardid=3&votes=[13,37]" | check_success
curl_post action.php "action=vote.cast&awardid=1&votes=[11,12]" | check_success

curl_post action.php "action=vote.cast&awardid=4&votes=[12,13]" | check_failure # Too many choices

# Delete the session cookie in order to change voters
rm $COOKIES_CURL

curl_post action.php "action=vote.cast&awardid=3&votes=[19,37]" | check_success

curl_getj "action.php?query=json.ballot.results" | \
    jq '.awards | .[].candidates | .[] | select(.score == 2).firstname' | \
    expect_eq '"Ian"'

curl_getj "action.php?query=json.ballot.results" | \
    jq '.awards | any( .candidates |  any( .carnumber == 136 ))' | \
    expect_eq false  # racer 36 (vote rejected)

curl_getj "action.php?query=json.ballot.results" | \
    jq '.awards | any( .candidates |  any( .carnumber == 242 ))' | \
    expect_eq false  # racer 42 (vote overwritten)

curl_post action.php "action=vote.cast&awardid=3&votes=[36]" | check_success
curl_getj "action.php?query=json.ballot.results" | \
    jq '.awards | any( .candidates |  any( .carnumber == 136 ))' | \
    expect_eq true

curl_post action.php "action=vote.cast&awardid=3&votes=[42]" | check_success
curl_getj "action.php?query=json.ballot.results" | \
    jq '.awards | any( .candidates |  any( .carnumber == 242 ))' | \
    expect_eq true
