#! /bin/bash
#
# This shared script provides definitions for several behaviors shared among the
# various photo scripts.

# If not disabled, the gvfs daemon grabs the connected camera before gphoto2 or
# chdkptp can.  The better solution is:
#    systemctl --user stop gvfs-daemon
#    systemctl --user mask gvfs-daemon
# during installation.
killall_gvfs_volume_monitor() {
    NOW=`date +%s`
    DEADLINE=`expr $NOW + 120`
    # Keep going for two minutes, in case the daemon hasn't yet come up when we
    # try to kill it.  We don't want to commit resources to running this process
    # indefinitely, though.
    while [ `date +%s` \< $DEADLINE ] ; do
        # gvfs-daemon is a user daemon; in order to kill it, we need to run
        # systemctl as the affected user, not root, so no sudo here.
        systemctl --user stop gvfs-daemon
        # There are actually a bunch of these gvfs-xxx-volume-monitor daemons, so this is likely not enough.  Stopping the gvfs-daemon user
        sudo killall gvfs-gphoto2-volume-monitor > /dev/null 2>&1
        sleep 4s
    done &
}

rotate_logs() {
    for LOG in uploads.log checkins.log ; do
        if [[ -s ${LOG} ]] ; then
            CDATE=$(date -d "1970-01-01 UTC + $(stat --format=%W ${LOG}) seconds" +"%Y-%m-%d-%H%M")
            mv ${LOG} ${LOG}.${CDATE}
        elif [[ -e ${LOG} ]] ; then
            rm ${LOG}
        fi
        # Purge year-old logs
        YEAR_AGO=$(date -d "-1 year" +"%Y-%m-%d-%H%M")
        for OLD in ${LOG}.* ; do
            if [[ -e ${OLD} && ${OLD} < ${LOG}.${YEAR_AGO} ]] ; then
                rm ${OLD}
            fi
        done
    done
}

check_for_barcode_quit() {
    read -t 0.5 BARCODE
    if [ "$BARCODE" = "QUITQUITQUIT" ] ; then
        announce terminating
        sudo shutdown -h now
    fi
}

# Test that we have a non-loopback network interface available
check_for_network() {
    ON_NETWORK=0
    while [ $ON_NETWORK -eq 0 ]; do
        check_for_barcode_quit
        ip -o address list | grep -v ' lo ' > /dev/null && ON_NETWORK=1
        if [ $ON_NETWORK -eq 0 ]; then
            echo Not on any network
            announce no-network
            sleep 5s
        fi
    done
}

# Try to log in to the derbynet server; keep trying until successful.
#
# This helps flush out connectivity problems and maybe password problems at the
# start of the script.
#
# Inputs from the environment:
#    ADJUST_CLOCK
#    DERBYNET_SERVER
#    PHOTO_USER
#    PHOTO_PASSWORD
#    COOKIES (path to cookie jar for both read and write)
do_login() {
    check_for_network

    # If there are connectivity problems, keep trying until login is successful.
    LOGIN_OK=0
    TMPFILE=`mktemp`
    while [ $LOGIN_OK -eq 0 ]; do
        check_for_barcode_quit
        echo Logging in to $DERBYNET_SERVER
        announce sending
        curl --location \
             --silent --show-error \
	     -b "$COOKIES" -c "$COOKIES" -o - \
	     --data "action=role.login&name=$PHOTO_USER&password=$PHOTO_PASSWORD" \
             "$DERBYNET_SERVER/action.php" \
            | tee "$TMPFILE" \
            | grep -q success \
            && LOGIN_OK=1
        announce idle
        test $LOGIN_OK -eq 0 && sleep 5s
    done

    if [ $ADJUST_CLOCK -ne 0 ]; then
        NEWTIME=`grep timecheck "$TMPFILE" | sed -e 's/.*: "\(.*\)".*/\1/'`
        echo Adjusting clock to $NEWTIME
        sudo date -s "$NEWTIME"
    fi
    echo Successfully logged in
    announce login-ok
}

# This should follow the do_login if we want the login's ADJUST_CLOCK behavior
# to affect the choice of photo directory.
define_photo_directory() {
    # Clean up old photo directories
    EXPIRATION_DATE=$(date -d "-2 weeks" +%Y-%m-%d)
    for D in photos-* ; do
        if [[ -d "${D}" && "${D}" < photos-${EXPIRATION_DATE} ]] ; then
	    rm -rf "${D}"
	fi
    done

    CUR_DIR="`pwd`"
    test -z "$PHOTO_DIR" && PHOTO_DIR="$CUR_DIR/photos-`date '+%Y-%m-%d'`"
    test ! -d "$PHOTO_DIR" && mkdir "$PHOTO_DIR"
}

# Echoes to stdout the full path of the barcode scanner input device, in /dev/input/by-id.
#
# If none of the names in $BARCODE_SCANNER_DEVS is found under /dev/input/by-id, this
# method falls back to ANY file under /dev/input/by-id, which is likely to include any
# attached keyboards or mice.
#
# Echoes an empty string to stdout if NO candidate devices are found.
#
# Input from the environment:
#    BARCODE_SCANNER_DEVS -- space-separated list of possible file names in /dev/input/by-id
find_barcode_scanner() {
    for DEV in $BARCODE_SCANNER_DEVS `ls /dev/input/by-id` ; do
        if [ -e "/dev/input/by-id/$DEV" ] ; then
            echo /dev/input/by-id/$DEV
            return
        fi
    done
}

# Verifies that the barcode scanner device is connected; loops until successful.
#
# Input from the environment:
#    BARCODE_SCANNER_DEVS
check_scanner() {
    # A barcode reader cabled to a pi will be seen like a local keyboard.  If
    # we're running the script over ssh, the script won't see scanned inputs.
    xhost >/dev/null 2>&1 || \
        (echo ; echo ; echo "NOT RUNNING UNDER X" ; \
         echo "LOCALLY-ATTACHED SCANNERS MAY NOT WORK" ; echo ; echo)

    ## This tries to confirm that there's a barcode scanner directly attached.
    ## The test is pretty sketchy, relying on the user having configured the
    ## identifier for their particular scanner(s) in the BARCODE_SCANNER_DEVS
    ## environment variable.  Since NOT having the scanner connected is
    ## something the user will easily detect for themselves the first time they
    ## attempt to use it, keepting this test seems not very worthwhile.

    return
    
    while [ -z "`find_barcode_scanner`" ] ; do
        echo Scanner not connected
        announce no-scanner
        sleep 5s
    done
}

# If using chdkptp, loop until there is a camera connected.
#
# Input from the environment:
#    PHOTO_CAPTURE
check_camera() {
    # Connect to camera, set to picture-taking mode.  (This lets operator adjust
    # photo composition.)
    #
    # Assumes there's only one camera attached
    if [ "$PHOTO_CAPTURE" = "chdkptp" ] ; then
        echo Checking for camera
        while [ -z  "`chdkptp -elist`" ] ; do
            announce no-camera
            sleep 5s
        done
        echo Activating camera
        chdkptp -c -e"rec"
        [ -x /usr/bin/flite ] && flite -t "Lights, camera, action"
    fi
}

# If configured to do so, ask web server to check in the current racer.
#
# Inputs from the environment:
#    PHOTO_CHECKIN (boolean)
#    BARCODE (string read by barcode scanner)
#    DERBYNET_SERVER
#    COOKIES
# Outputs:
#    CHECKIN_OK (boolean, 1=ok)
maybe_check_in_racer() {
    if [ $PHOTO_CHECKIN -ne 0 -a "$BARCODE" != "PWDuploadtest" -a "$BARCODE" != "PWDuploadonly" ] ; then
        TMPFILE=`mktemp`
        echo Checking in racer $BARCODE at `date` | tee -a checkins.log
        # Check in the racer
        CHECKIN_OK=0
        curl --location \
             --silent --show-error \
             -F action=racer.pass \
             -F barcode=$BARCODE \
             -F value=1 \
             -b "$COOKIES" -c "$COOKIES" \
             "$DERBYNET_SERVER/action.php" \
            | tee -a checkins.log \
            | tee "$TMPFILE" \
            | grep -q success && CHECKIN_OK=1
        if [ $CHECKIN_OK -eq 0 ] ; then
            echo Check-in failed | tee -a checkins.log
            tail checkins.log
            announce checkin-failed
        else
            if [ -x /usr/bin/flite ] ; then
                FIRSTNAME=`grep firstname "$TMPFILE" | sed -e 's/.*: "\(.*\)".*/\1/'`
                LASTNAME=`grep lastname "$TMPFILE" | sed -e 's/.*: "\(.*\)".*/\1/'`
                flite -t "Successful check in for $FIRSTNAME $LASTNAME"
            fi
        fi
        rm "$TMPFILE"
    else
        CHECKIN_OK=1
    fi
}

# Try to upload one photo to the web server.
#
# The path to the photo is passed as an argument.
#
# Other inputs from the environment:
#    BARCODE
#    PHOTO_REPO
#    AUTOCROP
#    COOKIES
#    DERBYNET_SERVER
#    CHECKIN_OK (boolean describing success of check-in attempt, if any; 1=success)
upload_photo() {
    PHOTO_PATH="$1"
    echo Uploading $PHOTO_PATH for $BARCODE at `date` | tee -a uploads.log
    announce sending
    UPLOAD_OK=0
    curl --location \
         --silent --show-error \
         --dump-header /dev/stdout \
         -b "$COOKIES" -c "$COOKIES" \
         -F action=photo.upload \
         -F MAX_FILE_SIZE=30000000 \
         -F repo=$PHOTO_REPO \
         -F barcode=$BARCODE \
         -F autocrop=$AUTOCROP \
         -F "photo=@$PHOTO_PATH;type=image/jpeg" \
         "$DERBYNET_SERVER/action.php" \
        | tee -a uploads.log \
        | grep -q success && UPLOAD_OK=1

    if [ $UPLOAD_OK -eq 1 ] ; then
        if [ $CHECKIN_OK -eq 1 ] ; then
            announce idle
            sleep 0.5s
            announce success
        else
            announce upload-ok-but-checkin-failed
        fi
    else
        echo Upload failed | tee -a uploads.log
        tail -n 15 uploads.log
        announce upload-failed
    fi
}

# Generate progressively larger local files of sorta-random bytes, and time the
# upload.
#
# Inputs from the environment:
#    COOKIES
#    DERBYNET_SERVER
upload_speed_test() {
    [ -x /usr/bin/flite ] && flite -t "Beginning speed test"
    RANDOM_JPG=upload-test.random.jpg
    COUNT=25
    BS=2048

    dd if=/dev/urandom of=$RANDOM_JPG bs=$BS count=$COUNT status=none

    echo | tee -a uploads.log
    echo | tee -a uploads.log

    # The web server is normally configured for 8M uploads; larger will just
    # give errors.
    while [ `expr $COUNT \* $BS \* 2` -lt 8000000 ] ; do
        START=`date +%s`
        # At larger sizes, /dev/urandom can take several seconds, so we just
        # reuse the bytes we've already got.
        dd if=$RANDOM_JPG of=$RANDOM_JPG \
           seek=$COUNT bs=$BS count=$COUNT status=none
        END=`date +%s`

        COUNT=`expr $COUNT + $COUNT`
        [ `expr $END - $START` -ne 0 ] && \
            echo `expr $END - $START` "second(s) to double file to" $COUNT bytes | tee -a uploads.log

        BARCODE=PWDuploadtest
        START=`date +%s`
        curl --fail \
             -F action=photo.upload \
             -F MAX_FILE_SIZE=30000000 \
             -F repo=$PHOTO_REPO \
             -F barcode=$BARCODE \
             -F autocrop=0 \
             -F "photo=@$RANDOM_JPG;type=image/jpeg" \
             -b "$COOKIES" -c "$COOKIES" \
             "$DERBYNET_SERVER/action.php"
        END=`date +%s`

        echo `expr $END - $START` "second(s) to upload" `expr $COUNT \* $BS` bytes | tee -a uploads.log
        if [ `expr $END - $START` -ne 0 ] ; then
            SPEED=`expr \( $COUNT \* $BS \) / \( $END - $START \)`
            KB_SPEED=`expr $SPEED / 1000`
            echo $SPEED bytes per second | tee -a uploads.log
            echo $KB_SPEED Kb per second | tee -a uploads.log
            if [ $KB_SPEED -gt 200 ] ; then
                announce speed-good
            elif [ $KB_SPEED -gt 50 ] ; then
                announce speed-fair
            else
                announce speed-poor
            fi
            # Abandon the test if the trials are becoming impractical
            [ `expr $END - $START` -gt 10 ] && return
        fi

        echo | tee -a uploads.log
        echo | tee -a uploads.log
    done

    rm $RANDOM_JPG
    announce success

    [ -x /usr/bin/flite ] && flite -t "$KB_SPEED kilobytes per second"
}
