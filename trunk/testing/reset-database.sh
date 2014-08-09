#! /bin/sh

BASE_URL=$1
source common.sh

./login-coordinator.sh $BASE_URL

curl_post action.php "action=run-sql&script=schema"

./import-roster.sh $BASE_URL
