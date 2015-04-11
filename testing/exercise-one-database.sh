#! /bin/sh

# Set to empty string to skip photo assignments
TEST_PHOTO_ASSIGNMENTS="YES"

run_tests() {
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-basic-racing.sh "$BASE_URL"
    `dirname $0`/test-new-rounds.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-photo-manipulations.sh "$BASE_URL"
    if [ -z "$TEST_PHOTO_ASSIGNMENTS" ]; then
        tput setaf 2  # green text
        echo "*** Skipping photo assignment tests"
        tput setaf 0  # black text
    else
        `dirname $0`/test-photo-assignments.sh "$BASE_URL"
        `dirname $0`/test-each-role.sh "$BASE_URL"
    fi

    `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/test-master-schedule.sh "$BASE_URL"
    `dirname $0`/test-new-rounds.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    if [ -z "$TEST_PHOTO_ASSIGNMENTS" ]; then
        tput setaf 2  # green text
        echo "*** Skipping photo assignment tests"
        tput setaf 0  # black text
    else
        `dirname $0`/test-photo-assignments.sh "$BASE_URL"
        `dirname $0`/test-each-role.sh "$BASE_URL"
    fi
}
