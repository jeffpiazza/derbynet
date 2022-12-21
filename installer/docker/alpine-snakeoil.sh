#! /bin/bash

apk add openssl

CONFIG=$(mktemp)

cat >$CONFIG <<EOF
[ req ]
default_bits            = 2048
default_keyfile         = privkey.pem
distinguished_name      = req_distinguished_name
prompt                  = no
policy                  = policy_anything
req_extensions          = v3_req
x509_extensions         = v3_req

[ req_distinguished_name ]
commonName                      = DerbyNet

[ v3_req ]
basicConstraints        = CA:FALSE
subjectAltName          = DNS:DerbyNetDockerImage
EOF


export RANDFILE=/dev/random

openssl req -config $CONFIG -new -x509 -days 3650 -nodes -sha256 \
	-out /etc/ssl/certs/ssl-cert-snakeoil.pem \
	-keyout /etc/ssl/private/ssl-cert-snakeoil.key

chmod 644 /etc/ssl/certs/ssl-cert-snakeoil.pem
chmod 640 /etc/ssl/private/ssl-cert-snakeoil.key

# chown root:ssl-cert /etc/ssl/private/ssl-cert-snakeoil.key

create_hash_link() {
    local file="$1"
    local cryptfile filename i
    filename=$(basename "$file")
    cryptfile=$(dirname "$file")/$(openssl x509 -hash -noout -in "$file")
    i=0
    while [ -L "${cryptfile}.$i" ] ; do
	if [ "$(readlink "${cryptfile}.$i")" = "$filename" ] ; then
	    return 0
	fi
	i=$(( i + 1 ))
    done
    ln -sf "$filename" "${cryptfile}.$i"
}

create_hash_link /etc/ssl/certs/ssl-cert-snakeoil.pem

# /etc/ssl/certs/ssl-cert-snakeoil.pem
