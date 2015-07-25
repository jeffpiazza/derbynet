#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

export SKIP_PUT_SNAPSHOT=1

curl_post setup-action.php \
    "connection_string=odbc:DSN=gprm;Exclusive=NO&dbuser=&dbpass=" \
    | check_success



