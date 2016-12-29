#! /bin/sh

# This script creates the /etc/apache2/derbynet directory, including
# configuration file and SSL certificates.

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

cat <<EOF | sed -e "s/SERVERNAME/$SERVERNAME/" >$DSTVOLUME/private/etc/apache2/derbynet/derbynet.conf
<Directory /Library/WebServer/Documents/derbynet>
    Otions FollowSymLinks Indexes MultiViews
    AllowOverride All
    Order allow,deny
    Allow from all
</Directory>

# Uncomment these sections to expose derbynet as the virtual host SERVERNAME, including SSL/HTTPS support.
#<VirtualHost *:80>
#    ServerName SERVERNAME
#    ServerAlias localhost
#    DocumentRoot /Library/WebServer/Documents/derbynet
#</VirtualHost>

#<VirtualHost *:443>
#    ServerName SERVERNAME
#    ServerAlias localhost
#    DocumentRoot /Library/WebServer/Documents/derbynet
#
#    SSLEngine on
#    SSLCipherSuite ALL:!ADH:!EXPORT56:RC4+RSA:+HIGH:+MEDIUM:+LOW:+SSLv2:+EXP:+eNULL
#    SSLCertificateFile /private/etc/apache2/derbynet/derbynet.crt
#    SSLCertificateKeyFile /private/etc/apache2/derbynet/derbynet.key
#</VirtualHost>
EOF
