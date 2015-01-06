#! /bin/sh

BASE_URL=$1
source `dirname $0`/common.sh

prepare_for_setup() {
# TODO: Simulate new database configuration by doing:
# Delete /Library/WebServer/Documents/xsite/local/config-database.inc
#  and /Library/WebServer/Documents/xsite/local/config-roles.inc
# and then delete the cookies file
`dirname $0`/login-coordinator.sh "$BASE_URL"
}

run_tests() {
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-basic-racing.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-photo-assignments.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

    `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/test-master-schedule.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-photo-assignments.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
}

prepare_for_setup
`dirname $0`/setup-mysql.sh "$BASE_URL"
run_tests

prepare_for_setup
`dirname $0`/setup-sqlite.sh "$BASE_URL"
run_tests
