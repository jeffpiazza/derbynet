#! /bin/sh

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

curl_post setup-action.php \
    "connection_string=sqlite:/Library/WebServer/Documents/xsite/local/trial.sqlite&dbuser=&dbpass=" \
    | check_success



