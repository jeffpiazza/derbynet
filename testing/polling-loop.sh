#! /bin/sh

# For load-testing the server, try nonstop poll.coordinator queries.  Run
# several threads to run simultaneously for a better load.

while true ; do
    `dirname $0`/query.sh "$1" query=poll.coordinator > /dev/null
done
