#! /bin/bash

run_tests() {
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

    `dirname $0`/test-basic-racing.sh "$BASE_URL"
    `dirname $0`/test-new-rounds.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-photo-manipulations.sh "$BASE_URL"
    `dirname $0`/test-photo-assignments.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

    `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/test-master-schedule.sh "$BASE_URL"
    `dirname $0`/test-new-rounds.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-photo-manipulations.sh "$BASE_URL"
    `dirname $0`/test-photo-assignments.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

    SNAPSHOT=$(mktemp /tmp/derby-snapshot.xml.XXXXX)
    curl_get "action.php?query=snapshot-tables" > $SNAPSHOT

    `dirname $0`/test-import-results.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

    curl_snapshot $SNAPSHOT | check_success
    rm $SNAPSHOT
}
