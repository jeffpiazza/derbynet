#! /bin/sh

BASE_URL=$1
source `dirname $0`/common.sh

curl_post setup-action.php "connection_string=sqlite:/Library/WebServer/Documents/xsite/local/trial.sqlite&dbuser=&dbpass=" | check_success

curl_post action.php "action=run-sql&script=schema" | check_success


