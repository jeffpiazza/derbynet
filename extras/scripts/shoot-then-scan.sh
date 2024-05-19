#! /bin/bash

# This script is a "shoot then scan" photo capture station.  gphoto2
# continuously captures photos from a tethered camera.  Whenever the barcode
# reader scans a barcode, it uploads the most recently captured photo to the
# server, identified by the barcode.
#

# Establish default values; photo-preamble will override from user config files.
#  Looks in /dev/input/by-id for any of the scanner devs, or just any device at all.
BARCODE_SCANNER_DEVS=""

SELF="$0"
# readlink -f exists in linux (particularly RPi), but not e.g. Mac
[ -L "$SELF" ] && SELF=`readlink -f "$SELF"`
SELF_DIR=`dirname "$SELF"`
LIB_DIR="$SELF_DIR/lib"

# Grab keyboard focus, assuming shoot-then-scan is the window title assigned by autostart file.
command -v wmctrl >/dev/null && wmctrl -a shoot-then-scan

. "$LIB_DIR"/photo-preamble.sh
. "$LIB_DIR"/photo-functions.sh

rotate_logs

killall_gvfs_volume_monitor

# The background task seems to continue running even after a control-C
# unless we clean it up. Next thing you know, your rpi's CPU is maxed out.
cleanup() {
    echo Cleanup "`basename $0`"
    killall "`basename $0`"
    exit 0
}
trap cleanup 2 3 6

# Depends on $PHOTO_DIR being defined when executed.
loop_to_capture_tethered() {
    EVER_FOUND_CAMERA=0

    HOOK_SCRIPT="`mktemp`"
    cat <<EOF >"$HOOK_SCRIPT"
#! /bin/sh
if [ "\$ACTION" = "download" ] ; then
    # For "announce" command definition:
    . "$LIB_DIR"/photo-preamble.sh
    . "$LIB_DIR"/photo-functions.sh

    # The file is already downloaded from the camera by the time 
    # this action comes
    >&2 echo Download action "\$ARGUMENT"
    ln -sf "\$ARGUMENT" "$CUR_DIR/last-photo.jpg"
    >&2 echo Link formed
    test -f /etc/derbynet.conf  && . /etc/derbynet.conf
    announce capture-ok &
elif [ "\$ACTION" = "start" ] ; then
    if [ $EVER_FOUND_CAMERA -eq 0 ] ; then
       >&2 echo Ready for my close-up
       test -x /usr/bin/flite && flite -t "Ready for my close-up"
       EVER_FOUND_CAMERA=1
    else
       >&2 echo hook script action start
    fi
else
    >&2 echo hook script action "\$ACTION"
fi
EOF
    chmod +x "$HOOK_SCRIPT"

    while true ; do
        GPHOTO2_OK=0
        # stdout gets flooded with a ton of messages like
        #     UNKNOWN PTP Property d1d3 changed
        # hence the /dev/null.
        gphoto2 --quiet --hook-script "$HOOK_SCRIPT" \
                --filename "$PHOTO_DIR/photo-%H%M%S-%n.%C" \
                --capture-tethered \
                >/dev/null && GPHOTO2_OK=1
        if [ $GPHOTO2_OK -eq 0 ] ; then
            announce no-camera
            sleep 5s
	fi
        sleep 1s
    done
}

do_login

define_photo_directory

check_scanner

announce idle

# Run the capture-tethered loop on a fork so camera capturing and photo uploads
# don't interfere with each other.
loop_to_capture_tethered &

while true ; do
    DEV="`find_barcode_scanner`"
    if [ -z "$DEV" ] ; then
        announce no-scanner
        sleep 5s
        continue
    fi
    read BARCODE
    echo Scanned $BARCODE
    CAR_NO=`echo $BARCODE | grep -e "^PWD" | sed -e "s/^PWD//"`

    if [ "$BARCODE" = "QUITQUITQUIT" ] ; then
        announce terminating
        sudo shutdown -h now
    elif [ "$BARCODE" = "PWDspeedtest" ] ; then
        upload_speed_test
    elif [ "$BARCODE" = "PWDcrop-OFF" ] ; then
        AUTOCROP=0
    elif [ "$BARCODE" = "PWDcrop-ON" ] ; then
        AUTOCROP=1
    elif [ ! -L last-photo.jpg -o ! -e "`readlink last-photo.jpg`" ] ; then
        # We'd like something more specific
        announce upload-failed
    # TODO Pack 1, 2024: Possibly this loop exited, or an upload_photo call hung, and
    # we kept capturing photos but became unable to upload them.  Workaround in either case
    # would be to reboot the station.
    elif [ "$CAR_NO" ] ; then
        # When the photo was captured, it had whatever name the camera assigned
        # it.  Now that we have a barcode value, we can rename the photo to
        # match the barcode value, in case the upload fails and manual
        # intervention is required later.
        LINK="`readlink last-photo.jpg`"
        DIRNAME="`dirname "$LINK"`"
        FILENAME=Car$CAR_NO.jpg
        I=0
        while [ "$LINK" != "$DIRNAME/$FILENAME" -a -e "$DIRNAME/$FILENAME" ] ; do
            I=`expr $I + 1`
            FILENAME=Car${CAR_NO}_$I.jpg
        done

        if [ "$LINK" != "$DIRNAME/$FILENAME" ] ; then
            mv "$LINK" "$DIRNAME/$FILENAME"
            ln -sf "$DIRNAME/$FILENAME" last-photo.jpg
        fi

        maybe_check_in_racer

        upload_photo "$DIRNAME/$FILENAME"

    else
        echo Rejecting scanned barcode $BARCODE
        announce unrecognized-barcode
    fi
done
