FROM nginx:stable-alpine
RUN apk add bash git
RUN echo -e "include /etc/nginx/derbynet/location.snippet;" > /etc/nginx/conf.d/default.conf
COPY prep_filesystem.sh /docker-entrypoint.d/.
RUN chmod 755 /docker-entrypoint.d/prep_filesystem.sh