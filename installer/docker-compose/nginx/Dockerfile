FROM nginx:stable-alpine

#Install all of the dependancies
RUN apk add bash git

#Insert our nginx config loader
RUN echo -e "include /etc/nginx/derbynet/location.snippet;" > /etc/nginx/conf.d/default.conf

#Copy the script into place which sets up the websites directory if it doesn't exist and then set the permissions on that file
COPY prep_filesystem.sh /docker-entrypoint.d/.
RUN chmod 755 /docker-entrypoint.d/prep_filesystem.sh