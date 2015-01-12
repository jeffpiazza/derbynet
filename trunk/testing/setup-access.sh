

TEST_PHOTO_ASSIGNMENTS=""

#! /bin/sh

BASE_URL=$1
source `dirname $0`/common.sh

curl_post setup-action.php \
    "connection_string=odbc:DSN=gprm;Exclusive=NO&dbuser=&dbpass=" \
    | check_success



