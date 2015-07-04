#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator
`dirname $0`/test-visit-each-page.sh $BASE_URL

user_login_crew
`dirname $0`/test-visit-each-page.sh $BASE_URL

user_logout
`dirname $0`/test-visit-each-page.sh $BASE_URL
