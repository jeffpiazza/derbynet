#! /bin/bash

# exercise-one-database.sh <base_url> sqlite [optional path to file]

BASE_URL=$1
shift
DBTYPE=$1
shift

set -e -E -o pipefail
source `dirname $0`/common.sh

prepare_for_setup() {
    # TODO: Simulate new database configuration by doing:
    # Delete /Library/WebServer/Documents/xsite/local/config-database.inc
    #  and /Library/WebServer/Documents/xsite/local/config-roles.inc
    # and then delete the cookies file
    user_login_coordinator
}

run_tests() {
############################## Basic Racing ##############################
    `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/import-roster.sh "$BASE_URL"
    `dirname $0`/test-den-changes.sh "$BASE_URL"

    `dirname $0`/photo-setup.sh "$BASE_URL"
    `dirname $0`/test-photo-upload.sh "$BASE_URL"

    `dirname $0`/test-basic-javascript.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-permissions.sh $BASE_URL

    `dirname $0`/test-basic-racing.sh "$BASE_URL"
    `dirname $0`/test-basic-javascript.sh "$BASE_URL"
    `dirname $0`/test-awards.sh "$BASE_URL" basic
    `dirname $0`/test-new-rounds.sh "$BASE_URL"
    `dirname $0`/test-basic-javascript.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"


    `dirname $0`/test-photo-manipulations.sh "$BASE_URL"
    `dirname $0`/test-photo-assignments.sh "$BASE_URL"
    `dirname $0`/test-photo-next.sh "$BASE_URL"

    `dirname $0`/test-racer-query.sh "$BASE_URL"

    `dirname $0`/test-extended-scheduling.sh "$BASE_URL"

############################## Standings by Rank ##############################
    `dirname $0`/test-standing-by-rank.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

############################## Points Racing ##############################
    `dirname $0`/test-points-racing.sh "$BASE_URL"

############################## One-Run-Per-Car Racing ##############################
    `dirname $0`/test-model-a-club.sh "$BASE_URL"

    `dirname $0`/test-messaging.sh "$BASE_URL"

############################## Master Schedule ##############################
    `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/import-roster.sh "$BASE_URL"
    `dirname $0`/test-den-changes.sh "$BASE_URL"
    `dirname $0`/test-master-schedule.sh "$BASE_URL"
    `dirname $0`/test-basic-javascript.sh "$BASE_URL"
    `dirname $0`/test-awards.sh "$BASE_URL" master
    `dirname $0`/test-new-rounds.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-photo-manipulations.sh "$BASE_URL"
    `dirname $0`/test-photo-assignments.sh "$BASE_URL"
    `dirname $0`/test-photo-upload.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

############################## Snapshot Export and Import ##############################
    `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/import-roster.sh "$BASE_URL"
    `dirname $0`/test-den-changes.sh "$BASE_URL"
    `dirname $0`/test-unused-lanes.sh "$BASE_URL"

    SNAPSHOT=$(mktemp /tmp/derby-snapshot.xml.XXXXX)
    curl_get "action.php?query=snapshot.get" > $SNAPSHOT

    `dirname $0`/test-import-results.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

    user_logout
    curl_put_snapshot $SNAPSHOT | check_failure

    if [ "x$SKIP_PUT_SNAPSHOT" == "x" ]; then
        user_login_coordinator
        curl_put_snapshot $SNAPSHOT | check_success
    else
        tput setaf 2  # green text
        echo Skipping snapshot put
        tput setaf 0  # black text
    fi

    rm $SNAPSHOT
}

rm `dirname $0`/*.curl 2>&1 || true

if [ "$DBTYPE" == "none" ] ; then
    run_tests
elif [ "$DBTYPE" == "sqlite" ] ; then
    DBPATH=${1:-/Library/WebServer/Documents/xsite/local/trial.sqlite}
    prepare_for_setup
    curl_post action.php \
        "action=setup.nodata&connection_string=sqlite:$DBPATH&dbuser=&dbpass=" \
        | check_success
    run_tests
elif [ "$DBTYPE" == "mysql" ] ; then
    echo DbType
    DBNAME=${1:-trial3}
    DBUSER=${2:-$DBNAME}
    DBPASS=${3:-}
    prepare_for_setup
    curl_post action.php \
              "action=setup.nodata&connection_string=mysql:host=localhost;dbname=$DBNAME&dbuser=$DBUSER&dbpass=$DBPASS" \
        | check_success
    run_tests
elif [ "$DBTYPE" == "access" ] ; then
    prepare_for_setup
    curl_post action.php \
              "action=setup.nodata&connection_string=odbc:DSN=gprm;Exclusive=NO&dbuser=&dbpass=" \
        | check_success

    # Access databases can't load a database snapshot, because it doesn't allow
    # primary key fields to be rewritten (I guess):
    #
    #  <exception>SQLSTATE[23000]: Integrity constraint violation: -1048
    #  [Microsoft][ODBC Microsoft Access Driver] Cannot update 'resultid'; field not
    #  updateable. (SQLExecute[-1048] at ext\pdo_odbc\odbc_stmt.c:254)</exception>

    export SKIP_PUT_SNAPSHOT=1
    run_tests
else
    tput setaf 1  # red text
    echo Unrecognized database type: $DBTYPE
    echo Known types are: sqlite mysql access
    tput setaf 0  # black text
    exit 1
fi
