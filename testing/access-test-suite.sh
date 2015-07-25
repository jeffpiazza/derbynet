#! /bin/bash
#
# Use this script to exercise the Access database, running on another (Windows) machine.

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh
source `dirname $0`/exercise-one-database.sh

prepare_for_setup() {
# TODO: Simulate new database configuration by doing:
# Delete /Library/WebServer/Documents/xsite/local/config-database.inc
#  and /Library/WebServer/Documents/xsite/local/config-roles.inc
# and then delete the cookies file
`dirname $0`/login-coordinator.sh "$BASE_URL"
}

prepare_for_setup
`dirname $0`/setup-access.sh "$BASE_URL"
run_tests
