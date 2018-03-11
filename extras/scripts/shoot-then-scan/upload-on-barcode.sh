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

do_login

check_scanner

while true ; do
    BARCODE=`barcode $BARCODE_SCANNER_DEV`
    echo Scanned $BARCODE
    announce barcode-read
    CAR_NO=`echo $BARCODE | grep -e "^PWD" | sed -e "s/^PWD//"`
    if [ "$CAR_NO" ] ; then

        maybe_check_in_racer

        upload_photo "`readlink last-photo.jpg`"

    elif [ "$BARCODE" = "QUITQUITQUIT" ] ; then
        announce terminating
        sudo shutdown -h now
    else
        echo Rejecting scanned barcode $BARCODE
        announce unrecognized-barcode
    fi
done
