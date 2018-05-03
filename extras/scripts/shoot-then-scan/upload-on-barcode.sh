#! /bin/sh

# This script is half of a "shoot then scan" photo capture station, this script
# being the "scan" half.  Whenever the barcode reader scans a barcode, it
# uploads the most recently captured photo to the server, identified by the
# barcode.

BARCODE_SCANNER_DEV=/dev/input/by-id/usb-13ba_Barcode_Reader-event-kbd

SELF="$0"
# readlink -f exists in linux (particularly RPi), but not e.g. Mac
[ -L "$SELF" ] && SELF=`readlink -f "$SELF"`
SELF_DIR=`dirname "$SELF"`
SUPER_DIR=`dirname "$SELF_DIR"`
. "$SUPER_DIR"/lib/photo-preamble.sh
. "$SUPER_DIR"/lib/photo-functions.sh

rm uploads.log > /dev/null 2>&1
rm checkins.log > /dev/null 2>&1

do_login

check_scanner

while true ; do
    BARCODE=`barcode $BARCODE_SCANNER_DEV`
    echo Scanned $BARCODE
    CAR_NO=`echo $BARCODE | grep -e "^PWD" | sed -e "s/^PWD//"`

    if [ "$BARCODE" = "QUITQUITQUIT" ] ; then
        announce terminating
        sudo shutdown -h now
    elif [ "$BARCODE" = "PWDspeedtest" ] ; then
        upload_speed_test
    elif [ ! -L last-photo.jpg -o ! -e "`readlink last-photo.jpg`" ] ; then
        # We'd like something more specific
        announce upload-failed
    elif [ "$CAR_NO" ] ; then

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
