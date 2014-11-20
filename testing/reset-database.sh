#! /bin/sh

BASE_URL=$1
source `dirname $0`/common.sh

`dirname $0`/login-coordinator.sh $BASE_URL

curl_post action.php "action=run-sql&script=schema" | check_success

`dirname $0`/import-roster.sh $BASE_URL
