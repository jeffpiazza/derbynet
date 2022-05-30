## docker build -t derbynet_server -f installer/docker/Dockerfile .

FROM ubuntu
LABEL org.opencontainers.image.authors=jeffpiazza
LABEL description="DerbyNet is the new standard in race management software for Pinewood Derby events.  \
Visit http://jeffpiazza.github.io/derbynet/ for more information about DerbyNet."

# I don't know when PHP picked up a dependency on tzdata and requires it be
# configured, but that seems to be the case now.  Eastern time is convenient for
# me, but may not be the best choice for everyone.
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update
RUN apt install -y php-fpm php-curl php-gd php-sqlite3
# php-odbc installs OK on a separate line, but not when added to the previous line.
# No idea why
RUN apt install -y php-odbc
RUN apt install -y nginx ssl-cert
RUN apt install -y supervisor

##################################################
# from debian/build.xml (debian-stage-website target):

ARG WWW_ROOT=/var/www/html

WORKDIR $WWW_ROOT

COPY website/ .
RUN rm -rf ./local/* ./index*.html

WORKDIR /etc/nginx/derbynet
COPY installer/nginx-configs/root/ .

# The set-timeout.sh script may be needed to prevent timeouts under automated testing.
# Under "normal" production use, stock timeouts are probably OK.
COPY installer/docker/set-timeout.sh /usr/local/bin

##################################################
# Post-install actions from Debian derbynet_server installer:

RUN test -d $WWW_ROOT/local && rm -rf $WWW_ROOT/local

RUN mkdir -m 777 /var/lib/derbynet
RUN echo '<?php $default_file_path = "/var/lib/derbynet"; ?>' > /var/lib/derbynet/default-file-path.inc

# How large are the pictures you want to upload?  8M covers an 18-megapixel
# photo.
ENV MAX_UPLOAD_SIZE=16M

RUN for INI in `find /etc/php -name php.ini` ; do \
    sed -i.pre-derbynet \
    -e "s#^upload_max_filesize = [28]M#upload_max_filesize = $MAX_UPLOAD_SIZE#" \
    -e "s#^post_max_size = [2-8]M#post_max_size = $MAX_UPLOAD_SIZE#" \
    -e "s#^memory_limit = 128M#memory_limit = 256M#" \
    -e "s#^session.gc_maxlifetime = 1440#session.gc_maxlifetime = 28800#" \
    $INI ; \
    done

RUN sed -i \
    -e "s#php.*-fpm.sock#php/php`ls /etc/php | tail -n 1`-fpm.sock#" \
    -e "s#CONFIG_DIR .*#CONFIG_DIR /var/lib/derbynet;#" \
    /etc/nginx/derbynet/location.snippet

RUN sed -i -e '/^[ \t]*location \/ {/ i\
  include derbynet/location.snippet; \

' /etc/nginx/sites-available/default

# This exposes the self-signed ("snakeoil") certificates for the nginx server,
# so a laptop user can just expose port 443.
#

# If instead you have a "legitimate" certifcate for a custom domain, you
# probably also already have a web server; in that case, configure your existing
# web server to proxy TLS derbynet requests through to the docker container's
# port 80, and don't bother exposting the docker container's 443 port.  E.g.,
# for nginx, assuming your derbynet_server container is listening on port 9000,
#
# location /my-derbynet {
#    proxy_pass  http://127.0.0.1:9000/;
# }
#
RUN sed -i \
    -e 's!# *\(listen 443 ssl .*\)!\1!' \
    -e 's!# *\(listen [::]:443 ssl .*\)!\1!' \
    -e 's!# *\(include snippets/snakeoil.conf;\)!\1!' \
    -e '/index\.php/! s!\( *index *\)!\1 index.php !' \
    /etc/nginx/sites-available/default

##################################################

VOLUME /var/lib/derbynet

EXPOSE 80 443

COPY installer/docker/supervisord.conf /etc/supervisor/conf.d/supervisord.conf

RUN sed -i \
   -e "s#@PHP-FPM@#$(ls /usr/sbin/php-fpm* | tail -n 1)#" \
   /etc/supervisor/conf.d/supervisord.conf

CMD ["/usr/bin/supervisord"]
