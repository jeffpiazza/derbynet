#! /bin/sh

# The DerbyNet installer enables MacOS' built-in Apache web server by enabling
# the MacOS launch daemon at
# /System/Library/LaunchDaemons/org.apache.httpd.plist.
#
# This command disables the web server (e.g., as a security measure):
#
sudo launchctl disable system/org.apache.httpd

sudo apachectl stop

