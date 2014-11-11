#! /bin/sh

BASE_URL=$1
source common.sh

user_login RaceCoordinator doyourbest
`dirname $0`/test-visit-each-page.sh $BASE_URL

user_login RaceCrew murphy
`dirname $0`/test-visit-each-page.sh $BASE_URL

user_logout
`dirname $0`/test-visit-each-page.sh $BASE_URL
