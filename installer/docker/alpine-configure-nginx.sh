#! /bin/bash


for INI in `find /etc/php* -name php.ini` ; do \
    sed -i.pre-derbynet \
    -e "s#^upload_max_filesize = [28]M#upload_max_filesize = $MAX_UPLOAD_SIZE#" \
    -e "s#^post_max_size = [2-8]M#post_max_size = $MAX_UPLOAD_SIZE#" \
    -e "s#^memory_limit = 128M#memory_limit = 256M#" \
    -e "s#^session.gc_maxlifetime = 1440#session.gc_maxlifetime = 28800#" \
    $INI ; \
done

    
    
sed -i -e "s#^listen *=.*#listen = /var/run/php/php-fpm.sock#" \
    /etc/php83/php-fpm.d/www.conf

sed -i \
    -e "s#fastcgi_pass.*#fastcgi_pass /var/run/php/php-fpm.sock#" \
    /etc/nginx/derbynet/location.snippet


## Rewrite the default nginx web site
cat >/etc/nginx/http.d/default.conf <<EOF

log_format derbynet_log
    '\$remote_addr - \$remote_user [\$time_local] '
    '"\$request" \$status '
    '\$body_bytes_sent "\$http_referer" '
    '"\$http_user_agent" '
    '[\$request_body]'
# Uncomment if investigating latency/performance:
#    ' rt=\$request_time'
#    ' ut=\$upstream_response_time'
;

# Competing access_log directives at the same level just perform more logging.
# Trying to turn off the default doesn't seem to work.  Instead, just override
# in the server directive.
#
# access_log off;

server {
	listen 80 default_server;
	listen [::]:80 default_server;

	# SSL configuration
	#
	listen 443 ssl default_server;
	listen [::]:443 ssl default_server;
	#
	# Note: You should disable gzip for SSL traffic.
	# See: https://bugs.debian.org/773332
	#
	# Read up on ssl_ciphers to ensure a secure configuration.
	# See: https://bugs.debian.org/765782
	#
    ssl_certificate /etc/ssl/certs/ssl-cert-snakeoil.pem;
    ssl_certificate_key /etc/ssl/private/ssl-cert-snakeoil.key;

    root /var/www/html;

	# Add index.php to the list if you are using PHP
	index  index.php index.html index.htm index.nginx-debian.html;

    access_log /var/log/nginx/access.log derbynet_log;

	server_name _;

    location ~ /.*\.php(/.*)?$ {
        client_max_body_size 16M;
        include fastcgi.conf;
        fastcgi_pass unix:/var/run/php/php-fpm.sock;
        fastcgi_param DERBYNET_CONFIG_DIR /var/lib/derbynet;
        fastcgi_param DERBYNET_DATA_DIR /var/lib/derbynet;
        # fastcgi_read_timeout value goes here, if needed.
  }


	location / {
		# First attempt to serve request as file, then
		# as directory, then fall back to displaying a 404.
		try_files \$uri \$uri/ =404;
	}
}
EOF









