#! /bin/sh


BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/login-coordinator.sh $BASE_URL
`dirname $0`/reset-database.sh "$BASE_URL"
curl_post action.php "action=write-settings&n-lanes=4" | check_success
`dirname $0`/checkin-all.sh "$BASE_URL"

curl_post action.php "action=class.edit&classid=1&name=Tigers" | check_success
curl_post action.php "action=class.edit&classid=2&name=Wolves" | check_success
curl_post action.php "action=class.edit&classid=3&name=Bears" | check_success
curl_post action.php "action=class.edit&classid=4&name=Webelos%20I" | check_success
curl_post action.php "action=class.edit&classid=5&name=Webelos%20II" | check_success

curl_post action.php "action=class.order&classid_1=1&classid_2=2&classid_3=3&classid_4=4&classid_5=5" | check_success

for r in 1 2 3 4 5; do
    curl_post action.php "action=schedule&roundid=$r" | check_success
    curl_post action.php "action=select-heat&now_racing=1&roundid=$r" | check_success

    while `dirname $0`/query.sh "$BASE_URL" query=watching | grep now-racing=\"1\" > /dev/null ; do
      echo Racing
      sleep 1s
    done

    sleep 5s
done
