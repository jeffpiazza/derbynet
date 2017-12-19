#! /bin/sh

# This script updates the configuration files for the built-in Apache server.
# It's tested at least to work with the Apache configuration as installed by
# macOS 10.11 (El Capitan).
#
# On a Mac that already has non-trivial configuration changes, this script may
# not have the desired effect, but it should be trivial to identify and correct
# any unwanted changes.

# This enables PHP processing by uncommenting two lines in the Apache web
# server's configuration file, then enables two other modules to support SSH
# handling.
sed -i.pre-derbynet \
  -e 's!# *\(LoadModule *php[57]_module .*\)!\1!' \
  -e 's!# *\(AddModule *mod_php5.c\)!\1!' \
  \
  -e 's!# *\(LoadModule *socache_shmcb_module .*\)!\1!' \
  -e 's!# *\(LoadModule ssl_module .*\)!\1!' \
  \
  "$DSTVOLUME/private/etc/apache2/httpd.conf"

grep -i derbynet.conf "$DSTVOLUME/private/etc/apache2/httpd.conf" > /dev/null || \
    echo '# Added by derbynet installer\nInclude /private/etc/apache2/derbynet/derbynet.conf' \
         >> "$DSTVOLUME/private/etc/apache2/httpd.conf"

# Restart Apache
/usr/sbin/apachectl restart
