#! /bin/sh

# For load-testing the server, try nonstop json.poll.coordinator queries.  Run
# several threads to run simultaneously for a better load.

while true ; do
    `dirname $0`/query.sh "$1" query=json.poll.coordinator > /dev/null
done
