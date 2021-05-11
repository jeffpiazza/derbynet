#! /bin/bash
#
# This exercises the rescheduler extensively.  It's not part of the regular test
# suite.

BASE_URL=$1
shift
DBTYPE=$1
shift

set -e -E -o pipefail
source `dirname $0`/common.sh


user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"

for i in $(seq 1 100) ; do
    curl_post action.php \
         "action=racer.import&firstname=Racer-$i&lastname=Racer-$i&classname=Unwashed-Class" | check_success
done
curl_post action.php "action=racer.bulk&what=number&who=all&start=101" | check_success

curl_post action.php "action=racer.import&firstname=RacerX&lastname=RacerX&classname=Unwashed-Class" | check_success
curl_post action.php "action=racer.import&firstname=RacerY&lastname=RacerY&classname=Unwashed-Class" | check_success


function setup_schedule() {
    NLANES="$1"
    NRACERS="$2"

    curl_post action.php "action=settings.write&n-lanes=$NLANES" | check_success
    curl_post action.php "action=racer.bulk&what=checkin&who=all&value=0" | check_success
    # NRACERS racers pass inspection
    for i in $(seq 1 $NRACERS) ; do
        curl_post action.php "action=racer.pass&racer=$i" | check_success
    done

    curl_postj action.php "action=json.schedule.generate&roundid=1" | check_jsuccess
    # RacerX
    curl_post action.php "action=racer.pass&racer=101&value=1" | check_success
    # RacerY
    curl_post action.php "action=racer.pass&racer=102&value=1" | check_success
    curl_postj action.php "action=json.heat.select&roundid=1&now_racing=1" | check_jsuccess
}


function do_reschedule() {
    echo
    curl_post action.php "action=schedule.reschedule&roundid=1"
    echo
}

function cleanup() {
    curl_postj action.php "action=json.result.delete&roundid=1" | check_jsuccess
    curl_postj action.php "action=json.schedule.unschedule&roundid=1" | check_jsuccess
}

function test_with_completions() {
    NLANES=4
    NRACERS="$1"
    NHEATS="$2"
    echo
    echo
    echo $NRACERS racers with $NHEATS completed "heat(s)"
    echo
    setup_schedule 4 $NRACERS
    if [ $NHEATS -gt 0 ] ; then
        for HEAT in $(seq 1 $NHEATS) ; do
            SKIP=""
            [ $HEAT -eq $NRACERS ] && SKIP=skip
            run_heat 1 $HEAT 2.0 2.5 3.0 3.5 $SKIP
        done
    fi
    do_reschedule
    cleanup
}

if true ; then
    for NRACERS in $(seq 2 5) ; do
        for NLANES in $(seq 2 6) ; do
            setup_schedule $NLANES $NRACERS
            do_reschedule
            cleanup
        done
    done
fi

if true ; then
    for NRACERS in $(seq 2 10) ; do
        for NHEATS in $(seq 0 $NRACERS) ; do
            test_with_completions $NRACERS $NHEATS
        done
    done
fi

if false ; then
    test_with_completions 2 1
    test_with_completions 2 2

    test_with_completions 3 1
    test_with_completions 3 2
    test_with_completions 3 3

    test_with_completions 4 2
    test_with_completions 4 3
    test_with_completions 4 4

    test_with_completions 5 4
fi
