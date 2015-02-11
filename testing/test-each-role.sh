#! /bin/sh

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login RaceCoordinator doyourbest
`dirname $0`/test-visit-each-page.sh $BASE_URL

user_login RaceCrew murphy
`dirname $0`/test-visit-each-page.sh $BASE_URL

user_logout
`dirname $0`/test-visit-each-page.sh $BASE_URL
