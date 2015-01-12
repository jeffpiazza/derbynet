#! /bin/sh

BASE_URL=$1
source `dirname $0`/common.sh

curl_post setup-action.php \
    "connection_string=mysql:host=localhost;dbname=trial3&dbuser=trial3&dbpass=" \
    | check_success


