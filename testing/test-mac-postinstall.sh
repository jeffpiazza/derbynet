#! /bin/sh

## maybe-create-local-directory.sh
##   $USER
##   $HOME
##   ** Tries to write /Library/WebServer/Documents/derbynet/

## create-apache-derbynet-directory.sh
##  Relies on `hostname`
## ** Calls security add-trusted-cert to install the new cert, but I think that just makes the cert trusted
##  from the hosting Mac.
## Writes $DSTVOLUME/private/etc/apache2/derbynet

## create-or-update-php-ini.sh
## ** Writes to /etc/php.ini and /var/log/php_errors

## update-apache-config.sh
##   $DSTVOLUME/private/etc/apache2/httpd.conf
##  ** /usr/sbin/apachectl restart

## maybe-rewrite-index-html.sh
## Tests md5 of 
##   $DSTVOLUME/Library/WebServer/Documents/index.html.en


