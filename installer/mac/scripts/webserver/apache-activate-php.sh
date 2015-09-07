#! /bin/sh

touch /private/etc/apache2/derbynet.receipt

# This enables PHP processing by uncommenting two lines in the Apache web server's configuration file
sed -i.pre-derbynet \
  -e 's!# *\(LoadModule *php5_module .*\)!\1!' \
  -e 's!# *\(AddModule *mod_php5.c\)!\1!' \
  "$DSTVOLUME/private/etc/apache2/httpd.conf"

# Restart Apache
/usr/sbin/apachectl restart
