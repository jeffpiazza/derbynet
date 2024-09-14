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
    #
    #user_login_coordinator
    # But it's OK if the login fails owing to there being no config file.
    PWD=$(sed -n -e "s/^RaceCoordinator://p" "${PASSWORDS_FILE:-`dirname $0`/default.passwords}")
	echo    >> $OUTPUT_CURL
	echo login RaceCoordinator >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
    SUCCESSFUL=1
	curl --location -d "action=role.login&name=RaceCoordinator&password=$PWD" \
        -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php | tee $DEBUG_CURL \
		| tee -a $OUTPUT_CURL | \
        jq -s -e ' length > 0 and (map(.outcome.summary == "success") | all)' >/dev/null || SUCCESSFUL=0
    if [ ! $SUCCESSFUL ] ; then
        tput setaf 1  # red text
        echo Initial login unsuccessful
    fi
    tput setaf 0  # black text

    # TODO Summarize success or failure here
}

function user_login_coordinator() {
    user_login RaceCoordinator
}

# $1 is the name of the .js file in puppeteer subdirectory
puppeteer_test() {
    echo
    echo " " " " " " $1
    node "`dirname $0`/puppeteer/$1" "$BASE_URL"
}

run_tests() {
############################## Basic Racing ##############################
    RESET_SOURCE=ex0 `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/import-roster.sh "$BASE_URL"
    `dirname $0`/test-den-changes.sh "$BASE_URL"

    `dirname $0`/photo-setup.sh "$BASE_URL"
    `dirname $0`/test-photo-upload.sh "$BASE_URL"

    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-permissions.sh $BASE_URL

    `dirname $0`/test-basic-racing.sh "$BASE_URL"

    puppeteer_test all-pages-test.js
    puppeteer_test coordinator-test.js
    puppeteer_test ondeck-columns-test.js

    `dirname $0`/test-awards.sh "$BASE_URL" basic
    `dirname $0`/test-new-rounds.sh "$BASE_URL"

    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-scenes.sh "$BASE_URL"

    `dirname $0`/test-photo-manipulations.sh "$BASE_URL"
    `dirname $0`/test-photo-assignments.sh "$BASE_URL"
    `dirname $0`/test-slide-next.sh "$BASE_URL"

    `dirname $0`/test-racer-query.sh "$BASE_URL"

    # Resets database
    `dirname $0`/test-car-numbers.sh "$BASE_URL"
    # Resets database
    `dirname $0`/test-extended-scheduling.sh "$BASE_URL"

    # Resets database
    `dirname $0`/test-reschedule.sh "$BASE_URL"
    # Resets database
    `dirname $0`/test-rotation-schedule.sh "$BASE_URL"

############################## Standings by Rank ##############################
    # Resets database
    `dirname $0`/test-standing-by-rank.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

############################## Points Racing ##############################
    # Resets database
    `dirname $0`/test-points-racing.sh "$BASE_URL"

############################## Rounds Playlist ##############################
    # Resets database
    `dirname $0`/test-playlist.sh "$BASE_URL"

############################## One-Run-Per-Car Racing ##############################
    `dirname $0`/test-model-a-club.sh "$BASE_URL"

    # Resets database
    `dirname $0`/test-messaging.sh "$BASE_URL"

############################## Racing Groups and Partitions ##############################
    RESET_SOURCE=ex1 `dirname $0`/reset-database.sh "$BASE_URL"
    puppeteer_test checkin-empty-test.js

    # Resets database
    `dirname $0`/test-partitions.sh "$BASE_URL"
    `dirname $0`/test-racing-groups.sh "$BASE_URL"
    # Resets database
    `dirname $0`/test-preferences.sh "$BASE_URL"

############################## Master Schedule ##############################
    RESET_SOURCE=ex2 `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/import-roster.sh "$BASE_URL"
    `dirname $0`/test-den-changes.sh "$BASE_URL"
    `dirname $0`/test-master-schedule.sh "$BASE_URL"

    `dirname $0`/test-awards.sh "$BASE_URL" master
    `dirname $0`/test-new-rounds.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"
    `dirname $0`/test-photo-manipulations.sh "$BASE_URL"
    `dirname $0`/test-photo-assignments.sh "$BASE_URL"
    `dirname $0`/test-photo-upload.sh "$BASE_URL"
    `dirname $0`/test-each-role.sh "$BASE_URL"

    # Resets database
    `dirname $0`/test-aggregate-rounds.sh "$BASE_URL"
    # Resets database
    `dirname $0`/test-aggregate-classes.sh "$BASE_URL"
    # Resets database
    `dirname $0`/test-aggregate-nonracing.sh "$BASE_URL"

############################## Snapshot Export and Import ##############################
    RESET_SOURCE=ex3 `dirname $0`/reset-database.sh "$BASE_URL"
    `dirname $0`/import-roster.sh "$BASE_URL"
    `dirname $0`/test-den-changes.sh "$BASE_URL"
    `dirname $0`/test-unused-lanes.sh "$BASE_URL"

    # Resets database
    `dirname $0`/test-balloting.sh "$BASE_URL"

    SNAPSHOT=$(mktemp /tmp/derby-snapshot.xml.XXXXX)
    echo Taking snapshot in $SNAPSHOT
    curl_get "action.php?query=snapshot.get" > $SNAPSHOT

    # Resets database
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

    tput setaf 2  # green text
    echo "<<<<<<<<<<<<<<<<<<<<<<<< Test run complete >>>>>>>>>>>>>>>>>>>>>>>>"
    tput setaf 0  # black text
}

rm `dirname $0`/*.curl 2>&1 || true

if [ "$DBTYPE" == "none" ] ; then
    run_tests
elif [ "$DBTYPE" == "ez" ] ; then
    prepare_for_setup
    curl_postj action.php "action=setup.nodata&ez-new=testdb" > /dev/null
    run_tests
elif [ "$DBTYPE" == "sqlite" ] ; then
    DBPATH=${1:-/Library/WebServer/Documents/xsite/local/trial.sqlite}
    prepare_for_setup
    curl_postj action.php \
        "action=setup.nodata&connection_string=sqlite:$DBPATH&dbuser=&dbpass=" \
        | check_jsuccess
    run_tests
elif [ "$DBTYPE" == "access" ] ; then
    prepare_for_setup
    curl_postj action.php \
              "action=setup.nodata&connection_string=odbc:DSN=gprm;Exclusive=NO&dbuser=&dbpass=" \
        | check_jsuccess

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
    echo Known types are: sqlite access ez none
    tput setaf 0  # black text
    exit 1
fi
