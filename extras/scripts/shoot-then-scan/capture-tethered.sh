#! /bin/sh

# This script is half of a "shoot then scan" photo capture station, this script
# being the "shoot" half.  Runs a loop downloading captured photos from a
# tethered camera, and storing them in a photo directory.  The symbolic link
# last-photo.jpg is updated to point to the most recent captured photo.
#
# Because we don't know the identity of the racer at the time the photo is
# taken, we name the photo with the date and time.


# Scripting hook to provides feedback to the user in response to various events
# encountered.  Takes one argument, the "event" name.
#
# Local installations should override by providing an alternate definition in
# /etc/derbynet.conf or similar:
#
#    #! /bin/sh
#    unset announce
#    announce() { ... }
#
announce() {
    echo Announce: $1
}

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

announce initializing

# Run a background task to kill gvfs-gphoto2-volume-monitor, in case
# it's still coming up when this script runs.
while true ; do sudo killall gvfs-gphoto2-volume-monitor > /dev/null 2>&1 ; sleep 4s ; done &
# The background task seems to continue running even after a control-C
# unless we clean it up. Next thing you know, your rpi's CPU is maxed out.
cleanup() {
    echo Cleanup "`basename $0`"
    killall "`basename $0`"
    exit 0
}
trap cleanup 2 3 6

HOOK_SCRIPT="`mktemp`"

CUR_DIR="`pwd`"
PHOTO_DIR="$CUR_DIR/photos-`date +%Y-%m-%d`"
mkdir "$PHOTO_DIR" >/dev/null 2>&1

cat <<EOF >"$HOOK_SCRIPT"
#! /bin/sh
if [ "\$ACTION" = "download" ] ; then
    # The file is already downloaded from the camera by the time 
    # this action comes
    >&2 echo Download action "\$ARGUMENT"
    ln -sf "\$ARGUMENT" "$CUR_DIR/last-photo.jpg"
    >&2 echo Link formed
    test -f /etc/derbynet.conf  && . /etc/derbynet.conf
    announce capture-ok &
else
    >&2 echo hook script action "\$ACTION"
fi
EOF
chmod +x "$HOOK_SCRIPT"

announce idle

while true ; do
    echo Top of loop
    GPHOTO2_OK=0
    # stdout gets flooded with a ton of messages like
    #     UNKNOWN PTP Property d1d3 changed
    # hence the /dev/null.
    gphoto2 --quiet --hook-script "$HOOK_SCRIPT" \
            --filename "$PHOTO_DIR/photo-%H%M%S-%n.%C" \
            --capture-tethered \
            >/dev/null && GPHOTO2_OK=1
    if [ $GPHOTO2_OK -eq 0 ] ; then
        announce no-camera &
    fi
    sleep 1s
done
