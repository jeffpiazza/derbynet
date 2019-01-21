#! /bin/sh
#
# This script uses uses a bar code scanner and a tethered camera to automate
# photo capture for your pinewood derby event.  See
# http://www.derbytalk.com/viewtopic.php?f=25&t=8253&p=79318 for more
# disccussion.
#
# Usage: photostand.sh <web server base URL>
# E.g.,  photostand.sh http://192.168.1.37/derbynet
#
# If the base URL is not given on the command line, and it's available from
# either /boot/derbynet.conf or /etc/derbynet.conf, that value will be used.
#
# The script logs in to the server when started, then loops waiting for barcodes
# to be scanned.  Check the server's php.ini, particularly
# session.gc_maxlifetime, to be sure that session cookies won't get reclaimed
# before we're done.
#
# Obtain the 'barcode' command:
#
# git clone https://github.com/jeffpiazza/read-barcode.git
# cd read-barcode
# make
# sudo cp barcode /usr/local/bin
#

BARCODE_SCANNER_DEV=/dev/input/by-id/usb-Megawin_Technology_Inc._USB_Keyboard-event-kbd

SELF="$0"
# readlink -f exists in linux (particularly RPi), but not e.g. Mac
[ -L "$SELF" ] && SELF=`readlink -f "$SELF"`
SELF_DIR=`dirname "$SELF"`
. "$SELF_DIR"/lib/photo-preamble.sh
. "$SELF_DIR"/lib/photo-functions.sh

rm uploads.log > /dev/null
rm checkins.log > /dev/null

killall_gvfs_volume_monitor

do_login

define_photo_directory

check_scanner

check_camera

while true ; do
    BARCODE=`barcode $BARCODE_SCANNER_DEV`
    echo Scanned $BARCODE
    CAR_NO=`echo $BARCODE | grep -e "^PWD" | sed -e "s/^PWD//"`
    if [ "$BARCODE" = "QUITQUITQUIT" ] ; then
        announce terminating
        sudo shutdown -h now
    elif [ "$BARCODE" = "PWDspeedtest" ] ; then
        upload_speed_test
    elif [ "$CAR_NO" ] ; then

        maybe_check_in_racer

        echo Capturing photo Car$CAR_NO.jpg
        if [ $USE_CHDKPTP -eq 0 ] ; then
            gphoto2 --filename "$PHOTO_DIR/Car$CAR_NO" --capture-image-and-download
        else
            prepare_camera_before_shot
            chdkptp -c -e"rec" -e"remoteshoot $PHOTO_DIR/Car$CAR_NO"
            echo chdkptp says $?
        fi

        announce capture-ok

        upload_photo "$PHOTO_DIR/Car$CAR_NO.jpg"

    else
        echo Rejecting scanned barcode $BARCODE
        announce unrecognized-barcode
    fi
done
