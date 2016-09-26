#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

if [ "$2" = "basic" ] ; then
    # Basic racing
    SECOND_TEST=' firstname="Jamison"'
    THIRD_TEST=' lastname="Burling"'
else
    # Master schedule racing
    SECOND_TEST=' firstname="Ben"'
    THIRD_TEST=' lastname="Halfacre"'
fi

user_login_coordinator

curl_post action.php "action=award.import" | check_failure
curl_post action.php "action=award.import&awardname=Test%20Award" | check_failure
curl_post action.php "action=award.import&awardname=Test%20Award&awardtype=Design" | check_failure
curl_post action.php "action=award.import&awardname=Test%20Award&awardtype=Design%20Trophy" | check_success
curl_post action.php "action=award.import&awardname=Test%20Den%20Award&classname=Hyenas&awardtype=Design%20Trophy" | check_failure
curl_post action.php "action=award.import&awardname=Test%20Den%20Award&classname=White's%20Wolves&awardtype=Design%20Trophy" | check_success
curl_post action.php "action=award.import&awardname=Test%20Rank%20Award&subgroup=White's%20Wolves&awardtype=Design%20Trophy&racerid=35" | check_success
curl_post action.php "action=award.import&awardname=Test%20Pack%20Award&awardtype=Design%20Trophy&carnumber=222&sort=6" | check_success

# The numbering of awardids depends critically on what awards were already
# present when the award was registered.
curl_post action.php "action=award.present&key=award-3" | check_success
curl_get "action.php?query=award.current" | expect_count '<award ' 1
curl_get "action.php?query=award.current" | expect_count 'Howell' 1
curl_get "action.php?query=award.current" | expect_count ' reveal="false"' 1


# There's no current award at the very beginning, but we don't want to enforce
# that this test has to be run at the very beginning.
#
# curl_get "action.php?query=award.current" | check_failure

curl_post action.php "action=award.present&key=speed-2" | check_success
curl_get "action.php?query=award.current" | expect_count '<award ' 1
curl_get "action.php?query=award.current" | expect_count ' reveal="false"' 1

curl_post action.php "action=award.present&reveal=1" | check_success
curl_get "action.php?query=award.current" | expect_count '<award ' 1
curl_get "action.php?query=award.current" | expect_count ' reveal="true"' 1
curl_get "action.php?query=award.current" | expect_count "$SECOND_TEST" 1

curl_post action.php "action=award.present&reveal=0&key=speed-3" | check_success
curl_get "action.php?query=award.current" | expect_count '<award ' 1
curl_get "action.php?query=award.current" | expect_count ' reveal="false"' 1
curl_get "action.php?query=award.current" | expect_count "$THIRD_TEST" 1

