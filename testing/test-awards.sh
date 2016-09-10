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

# TODO Test creating, assigning, and presenting a non-speed award
