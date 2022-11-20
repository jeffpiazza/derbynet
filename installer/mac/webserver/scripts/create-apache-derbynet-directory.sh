#! /bin/sh

# This script creates the /etc/apache2/derbynet directory, including
# configuration file and SSL certificates.

SERVERNAME="`hostname`"

mkdir "$DSTVOLUME/private/etc/apache2/derbynet"

if [ ! -f "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.crt" ] ; then
    rm -f "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.key"
    ssh-keygen -P "" -m PEM -f "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.key"
    openssl req -new -subj "/C=US/ST=-/O=-/localityName=-/CN=$SERVERNAME/"\
            -key "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.key" \
            -out "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.csr"
    # Per https://support.apple.com/en-us/HT210176, certificates need to be
    # valid for 825 days or fewer.
    openssl x509 -req -days 800 \
            -in "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.csr" \
            -signkey "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.key" \
            -out "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.crt"

    rm "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.csr"
fi

security add-trusted-cert -d -r trustRoot -k /Library/Keychains/System.keychain \
         "$DSTVOLUME/private/etc/apache2/derbynet/derbynet.crt"


cat <<EOF | sed -e "s/SERVERNAME/$SERVERNAME/" >$DSTVOLUME/private/etc/apache2/derbynet/derbynet.conf
SetEnv DERBYNET_CONFIG_DIR /Library/WebServer/Documents/derbynet/local
SetEnv DERBYNET_DATA_DIR   $HOME/Public/Derbynet

<Directory /Library/WebServer/Documents/derbynet>
    Options FollowSymLinks Indexes MultiViews
    AllowOverride All
    Order allow,deny
    Allow from all
</Directory>

Listen 443
<VirtualHost *:443>
    ServerName SERVERNAME
    ServerAlias localhost

    SSLEngine on
    SSLCipherSuite ALL:!ADH:!EXPORT56:RC4+RSA:+HIGH:+MEDIUM:+LOW:+SSLv2:+EXP:+eNULL
    SSLCertificateFile /private/etc/apache2/derbynet/derbynet.crt
    SSLCertificateKeyFile /private/etc/apache2/derbynet/derbynet.key
</VirtualHost>
EOF
