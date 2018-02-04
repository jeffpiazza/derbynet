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

HOOK_SCRIPT="`mktemp`"

CUR_DIR="`pwd`"
PHOTO_DIR="$CUR_DIR/photos-`date +%Y-%m-%d`"
mkdir "$PHOTO_DIR" >/dev/null 2>&1

cat <<EOF >"$HOOK_SCRIPT"
#! /bin/sh
if [ "\$ACTION" = "download" ] ; then
    test -f /etc/derbynet.conf  && . /etc/derbynet.conf
    ln -sf "\$ARGUMENT" "$CUR_DIR/last-photo.jpg"
    announce capture-ok
fi
EOF
chmod +x "$HOOK_SCRIPT"

while true ; do
    gphoto2 --quiet --hook-script "$HOOK_SCRIPT" \
            --filename "$PHOTO_DIR/photo-%H%M%S-%n.%C" \
            --capture-tethered \
            >/dev/null
    sleep 1s
done
