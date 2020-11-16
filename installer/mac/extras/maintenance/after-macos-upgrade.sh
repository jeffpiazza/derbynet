#! /bin/sh

################################################################################
#
# A MacOS upgrade (i.e., installing a new version of MacOS) typically rewrites
# the configuration files for MacOS' built-in web server, causing DerbyNet to
# stop working.  This script re-applies the changes made by the DerbyNet
# installer to enable PHP in Apache.
#
# This script gets invoked at start-up by
# /Library/LaunchDaemons/org.jeffpiazza.derbynet.after-macos-upgrade.sh
#
################################################################################

if grep derbynet /etc/apache2/httpd.conf > /dev/null ; then
    :
else
    `dirname $0`/create-or-update-php-ini.sh
    `dirname $0`/update-apache-config.sh
fi
