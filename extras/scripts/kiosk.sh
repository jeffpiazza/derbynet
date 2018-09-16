#! /bin/sh

# This script attempts to present a full-screen browser pointed at the DerbyNet
# web server.  Place the companion kiosk.desktop file in ~/.config/autostart/ to
# have it autostart.
#
# Supports midori and epiphany browsers, commonly found on Raspbian.  Midori is
# the default; override with a BROWSER= value in one of the config files
# (/etc/derbynet.conf or /boot/derbynet.conf).
#
# Note that Midori is not installed by default:
#
# sudo apt-get install midori
#
# Epiphany is the default browser for Raspbian, but it doesn't seem to support a
# "full screen" mode that clears the desktop decorations (menu bar, etc.).  The
# work-around is to install xautomation (which provides the xte command), and
# simulate an F11 keystroke to put the browser in fullscreen mode.


xset s off
xset -dpms
xset s noblank
test -x /usr/bin/unclutter && unclutter &

# Choosing the X font is kind of a nuisance; getting it wrong may result in a dialog box with no visible text.
# See https://tronche.com/wiki/Howto_specify_X_scalable_fonts_from_the_command_line
#
# XFONT='-*-*-*-r-*--40-0-0-0-p-*-*-*'
XFONT='-adobe-helvetica-*-r-*--40-0-0-0-p-*-iso8859-1'

# NOTE: A warning from xmessage of
#    Cannot convert string "vlines2" to type Pixmap
# means you need to install package xbitmaps

# A blank LC_ALL value yields the xmessage warning:
#    Missing charsets in String to FontSet conversion
if [ "$LC_ALL" = "" ] ; then
    export LC_ALL=C
fi

test -x /usr/bin/xmessage && xmessage -fn "$XFONT" -buttons '' -center -timeout 2 -file - <<EOF &


         Initializing, please wait.


EOF

# Default browser; last one wins
BROWSER=epiphany
test -x /usr/bin/midori && BROWSER=midori
test -x /usr/bin/chromium && BROWSER=/chromium
test -x /usr/bin/chromium-browser && BROWSER=chromium-browser

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

test -n "$1" && DERBYNET_SERVER="$1"

# If the browser crashes, we'd like to resume with the same identity, rather
# than as a new browser/kiosk.  We use the CPU serial number to identify
# ourselves.
if [ -z "$ADDRESS" -a -f /proc/cpuinfo ] ; then
    ADDRESS=$(sed -n -e 's/^Serial[ \t:]*0*\(.*\)/\1/p' /proc/cpuinfo)
fi

# Avoid two simultaneous xmessage displays
sleep 2s

# Ensure the networking stack is functional, because the browser will just
# present an error screen if it gets a "no route to host."

CONTACT_OK=0
while [ $CONTACT_OK -eq 0 ]; do
    test -x /usr/bin/xmessage && xmessage -fn "$XFONT" -buttons '' -center -timeout 5 -file - <<EOF &


    Attempting to contact web server
    at $DERBYNET_SERVER.
    Please wait.


EOF
    curl --connect-timeout 6 --location --silent "$DERBYNET_SERVER/index.php" > /dev/null && CONTACT_OK=1
done

# While loop allows recovery from browser crashes
while true ; do
    test -x /usr/bin/logger && logger -t kiosk Starting $BROWSER browser
    if [ "$BROWSER" = "midori" ] ; then
        midori --execute Fullscreen --app "$DERBYNET_SERVER/kiosk.php?address=$ADDRESS"
    elif [ "$BROWSER" = "epiphany" ] ; then
        # If xautomation is present, use xte to put the browser into fullscreen mode.
        # The sleep is to allow the browser to get set up.
        test -x /usr/bin/xte && xte 'sleep 15' 'key F11' &
        epiphany-browser --application-mode --profile /home/pi/.config "$DERBYNET_SERVER/kiosk.php?address=$ADDRESS"
    elif [ "$BROWSER" = "chromium"  -o "$BROWSER" = "chromium-browser" ] ; then
        # From https://www.danpurdy.co.uk/wp-content/cache/page_enhanced/www.danpurdy.co.uk/web-development/raspberry-pi-kiosk-screen-tutorial/_index.html
        sed -i 's/"exited_cleanly": false/"exited_cleanly": true/' \
            ~/.config/chromium/Default/Preferences
        "$BROWSER" --noerrdialogs --kiosk "$DERBYNET_SERVER/kiosk.php?address=$ADDRESS" –incognito
    else
        # Arbitrary browser command:
        "$BROWSER" "$DERBYNET_SERVER/kiosk.php?address=$ADDRESS"
    fi
done
