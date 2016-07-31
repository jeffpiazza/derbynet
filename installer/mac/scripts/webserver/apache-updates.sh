#! /bin/sh

## TODO This needs to replace apache-activate-php.sh, and get referenced from the rest of the installer.

# This script updates the configuration files for the built-in Apache server.
# It's tested at least to work with the Apache configuration as installed by
# macOS 10.11 (El Capitan).
#
# On a Mac that already has non-trivial configuration changes, this script may
# not have the desired effect, but it should be trivial to identify and correct
# any unwanted changes.

SERVERNAME=derby.net

mkdir "$DSTVOLUME/private/etc/apache2/derbynet"

if [ ! -f "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.key" ] ; then
    ssh-keygen -P "" -f "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.key"
    openssl req -new -subj "/C=US/ST=-/O=-/localityName=-/CN=$SERVERNAME/"\
            -key "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.key" \
            -out "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.csr"
    openssl x509 -req -days 3650 \
            -in "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.csr" \
            -signkey "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.key" \
            -out "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.crt"

    rm "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.csr"
fi

# This enables PHP processing by uncommenting two lines in the Apache web
# server's configuration file, then enables two other modules to support SSH
# handling.
sed -i.pre-derbynet \
  -e 's!# *\(LoadModule *php5_module .*\)!\1!' \
  -e 's!# *\(AddModule *mod_php5.c\)!\1!' \
  \
  -e 's!# *\(LoadModule *socache_shmcb_module .*\)!\1!' \
  -e 's!# *\(LoadModule ssl_module .*\)!\1!' \
  \
  "$DSTVOLUME/private/etc/apache2/httpd.conf"

grep -i NameVirtualHost "$DSTVOLUME/private/etc/apache2/httpd.conf" > /dev/null || \
    echo '# Added by derbynet installer\nNameVirtualHost *:80\nNameVirtualHost *:443' \
         >> "$DSTVOLUME/private/etc/apache2/httpd.conf"

grep -i derbynet.conf "$DSTVOLUME/private/etc/apache2/httpd.conf" > /dev/null || \
    echo 'Include /private/etc/apache2/derbynet/derbynet.conf' >> "$DSTVOLUME/private/etc/apache2/httpd.conf"

cat <<EOF | sed -e "s/SERVERNAME/$SERVERNAME/" >$DSTVOLUME/private/etc/apache2/derbynet/derbynet.conf
<VirtualHost *:80>
    ServerName SERVERNAME
    ServerAlias localhost
    DocumentRoot /Library/WebServer/Documents/derbynet

    <Directory /Library/WebServer/Documents/derbynet>
		Options FollowSymLinks Indexes MultiViews
		AllowOverride All
		Order allow,deny
		Allow from all
	</Directory>
</VirtualHost>

<VirtualHost *:443>
    ServerName SERVERNAME
    ServerAlias localhost
    DocumentRoot /Library/WebServer/Documents/derbynet

    SSLEngine on
    SSLCipherSuite ALL:!ADH:!EXPORT56:RC4+RSA:+HIGH:+MEDIUM:+LOW:+SSLv2:+EXP:+eNULL
    SSLCertificateFile /private/etc/apache2/derbynet/derbynet.crt
    SSLCertificateKeyFile /private/etc/apache2/derbynet/derbynet.key
</VirtualHost>
EOF

# Restart Apache
/usr/sbin/apachectl restart
