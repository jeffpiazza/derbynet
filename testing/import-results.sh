#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/login-coordinator.sh $BASE_URL

curl_post action.php "action=run-sql&script=schema" | check_success
curl_post action.php "action=run-sql&script=update-schema" | check_success

curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=1&lane=1&firstname=Matthew&lastname=Glennon&carnumber=105&finishtime=3.936&finishplace=3&completed=2015-03-08%2018:35:44" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=1&lane=3&firstname=Burt&lastname=Desrosier&carnumber=303&finishtime=3.313&finishplace=2&completed=2015-03-08%2018:35:44" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=1&lane=4&firstname=Matthew&lastname=Kinne&carnumber=107&finishtime=3.272&finishplace=1&completed=2015-03-08%2018:35:44" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=2&lane=1&firstname=Matthew&lastname=Kinne&carnumber=107&finishtime=3.256&finishplace=1&completed=2015-03-08%2018:37:41" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=2&lane=2&firstname=Matthew&lastname=Glennon&carnumber=105&finishtime=3.884&finishplace=3&completed=2015-03-08%2018:37:41" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=2&lane=4&firstname=Burt&lastname=Desrosier&carnumber=303&finishtime=3.333&finishplace=2&completed=2015-03-08%2018:37:41" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=3&lane=1&firstname=Burt&lastname=Desrosier&carnumber=303&finishtime=3.332&finishplace=2&completed=2015-03-08%2018:39:08" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=3&lane=2&firstname=Matthew&lastname=Kinne&carnumber=107&finishtime=3.271&finishplace=1&completed=2015-03-08%2018:39:08" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=3&lane=3&firstname=Matthew&lastname=Glennon&carnumber=105&finishtime=4.103&finishplace=3&completed=2015-03-08%2018:39:08" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=4&lane=2&firstname=Burt&lastname=Desrosier&carnumber=303&finishtime=3.343&finishplace=2&completed=2015-03-08%2018:40:16" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=4&lane=3&firstname=Matthew&lastname=Kinne&carnumber=107&finishtime=3.262&finishplace=1&completed=2015-03-08%2018:40:16" | check_success
curl_post action.php "action=import-one-result&class=Tiger&round=1&heat=4&lane=4&firstname=Matthew&lastname=Glennon&carnumber=105&finishtime=4.1&finishplace=3&completed=2015-03-08%2018:40:16" | check_success

curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=1&lane=1&firstname=Spencer&lastname=Vanhoose&carnumber=207&finishtime=3.206&finishplace=1&completed=2015-03-08%2018:42:08" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=1&lane=2&firstname=Evan&lastname=Saltsman&carnumber=203&finishtime=3.465&finishplace=4&completed=2015-03-08%2018:42:08" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=1&lane=3&firstname=Andrew&lastname=Rayes&carnumber=209&finishtime=3.32&finishplace=2&completed=2015-03-08%2018:42:08" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=1&lane=4&firstname=Bennett&lastname=Largo&carnumber=206&finishtime=3.347&finishplace=3&completed=2015-03-08%2018:42:08" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=2&lane=1&firstname=Matthew&lastname=Shupp&carnumber=211&finishtime=9.999&finishplace=4&completed=2015-03-08%2018:43:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=2&lane=2&firstname=Alexander&lastname=Balderas&carnumber=210&finishtime=3.255&finishplace=2&completed=2015-03-08%2018:43:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=2&lane=3&firstname=Cameron&lastname=McCollough&carnumber=205&finishtime=4.097&finishplace=3&completed=2015-03-08%2018:43:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=2&lane=4&firstname=Spencer&lastname=Vanhoose&carnumber=207&finishtime=3.182&finishplace=1&completed=2015-03-08%2018:43:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=3&lane=1&firstname=Andrew&lastname=Rayes&carnumber=209&finishtime=3.316&finishplace=2&completed=2015-03-08%2018:45:15" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=3&lane=2&firstname=Hayden&lastname=McCollough&carnumber=204&finishtime=3.757&finishplace=4&completed=2015-03-08%2018:45:15" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=3&lane=3&firstname=Dylan&lastname=Fukuda&carnumber=208&finishtime=3.375&finishplace=3&completed=2015-03-08%2018:45:15" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=3&lane=4&firstname=Alexander&lastname=Balderas&carnumber=210&finishtime=3.25&finishplace=1&completed=2015-03-08%2018:45:15" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=4&lane=1&firstname=Alex&lastname=Karim&carnumber=202&finishtime=3.312&finishplace=1&completed=2015-03-08%2018:47:21" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=4&lane=2&firstname=Bennett&lastname=Largo&carnumber=206&finishtime=3.331&finishplace=2&completed=2015-03-08%2018:47:21" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=4&lane=3&firstname=Matthew&lastname=Shupp&carnumber=211&finishtime=3.843&finishplace=4&completed=2015-03-08%2018:47:21" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=4&lane=4&firstname=Hayden&lastname=McCollough&carnumber=204&finishtime=3.764&finishplace=3&completed=2015-03-08%2018:47:21" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=5&lane=1&firstname=Alexander&lastname=Balderas&carnumber=210&finishtime=3.296&finishplace=2&completed=2015-03-08%2018:49:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=5&lane=2&firstname=Cameron&lastname=McCollough&carnumber=205&finishtime=4.376&finishplace=4&completed=2015-03-08%2018:49:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=5&lane=3&firstname=Alex&lastname=Karim&carnumber=202&finishtime=3.296&finishplace=1&completed=2015-03-08%2018:49:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=5&lane=4&firstname=Evan&lastname=Saltsman&carnumber=203&finishtime=3.508&finishplace=3&completed=2015-03-08%2018:49:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=6&lane=1&firstname=Hayden&lastname=McCollough&carnumber=204&finishtime=3.823&finishplace=3&completed=2015-03-08%2018:52:01" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=6&lane=2&firstname=Dylan&lastname=Fukuda&carnumber=208&finishtime=3.322&finishplace=2&completed=2015-03-08%2018:52:01" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=6&lane=3&firstname=Spencer&lastname=Vanhoose&carnumber=207&finishtime=3.178&finishplace=1&completed=2015-03-08%2018:52:01" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=6&lane=4&firstname=Cameron&lastname=McCollough&carnumber=205&finishtime=4.42&finishplace=4&completed=2015-03-08%2018:52:01" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=7&lane=1&firstname=Evan&lastname=Saltsman&carnumber=203&finishtime=3.582&finishplace=2&completed=2015-03-08%2018:53:28" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=7&lane=2&firstname=Andrew&lastname=Rayes&carnumber=209&finishtime=3.303&finishplace=1&completed=2015-03-08%2018:53:28" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=7&lane=3&firstname=Hayden&lastname=McCollough&carnumber=204&finishtime=3.952&finishplace=4&completed=2015-03-08%2018:53:28" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=7&lane=4&firstname=Matthew&lastname=Shupp&carnumber=211&finishtime=3.949&finishplace=3&completed=2015-03-08%2018:53:28" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=8&lane=1&firstname=Cameron&lastname=McCollough&carnumber=205&finishtime=4.132&finishplace=4&completed=2015-03-08%2018:54:39" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=8&lane=2&firstname=Alex&lastname=Karim&carnumber=202&finishtime=3.323&finishplace=2&completed=2015-03-08%2018:54:39" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=8&lane=3&firstname=Bennett&lastname=Largo&carnumber=206&finishtime=3.29&finishplace=1&completed=2015-03-08%2018:54:39" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=8&lane=4&firstname=Andrew&lastname=Rayes&carnumber=209&finishtime=3.328&finishplace=3&completed=2015-03-08%2018:54:39" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=9&lane=1&firstname=Dylan&lastname=Fukuda&carnumber=208&finishtime=3.295&finishplace=2&completed=2015-03-08%2018:55:57" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=9&lane=2&firstname=Spencer&lastname=Vanhoose&carnumber=207&finishtime=3.249&finishplace=1&completed=2015-03-08%2018:55:57" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=9&lane=3&firstname=Evan&lastname=Saltsman&carnumber=203&finishtime=3.449&finishplace=4&completed=2015-03-08%2018:55:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=9&lane=4&firstname=Alex&lastname=Karim&carnumber=202&finishtime=3.328&finishplace=3&completed=2015-03-08%2018:55:58" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=10&lane=1&firstname=Bennett&lastname=Largo&carnumber=206&finishtime=3.298&finishplace=2&completed=2015-03-08%2018:57:26" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=10&lane=2&firstname=Matthew&lastname=Shupp&carnumber=211&finishtime=3.881&finishplace=4&completed=2015-03-08%2018:57:26" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=10&lane=3&firstname=Alexander&lastname=Balderas&carnumber=210&finishtime=3.244&finishplace=1&completed=2015-03-08%2018:57:26" | check_success
curl_post action.php "action=import-one-result&class=Wolf&round=1&heat=10&lane=4&firstname=Dylan&lastname=Fukuda&carnumber=208&finishtime=3.367&finishplace=3&completed=2015-03-08%2018:57:26" | check_success

curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=1&lane=1&firstname=Gaurav&lastname=Gros&carnumber=409&finishtime=3.186&finishplace=1&completed=2015-03-08%2019:02:15" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=1&lane=2&firstname=Benjamin&lastname=Lisle&carnumber=407&finishtime=3.19&finishplace=2&completed=2015-03-08%2019:02:15" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=1&lane=3&firstname=Grayson&lastname=Shealey&carnumber=403&finishtime=3.252&finishplace=3&completed=2015-03-08%2019:02:15" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=1&lane=4&firstname=Evan&lastname=Largo&carnumber=401&finishtime=3.548&finishplace=4&completed=2015-03-08%2019:02:15" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=2&lane=1&firstname=Alexander&lastname=Lisle&carnumber=406&finishtime=3.345&finishplace=3&completed=2015-03-08%2019:03:33" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=2&lane=2&firstname=Zachary&lastname=Strawser&carnumber=402&finishtime=3.164&finishplace=1&completed=2015-03-08%2019:03:33" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=2&lane=3&firstname=Henry&lastname=Henley&carnumber=405&finishtime=3.37&finishplace=4&completed=2015-03-08%2019:03:33" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=2&lane=4&firstname=Gaurav&lastname=Gros&carnumber=409&finishtime=3.244&finishplace=2&completed=2015-03-08%2019:03:33" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=3&lane=1&firstname=Benjamin&lastname=Lisle&carnumber=407&finishtime=3.229&finishplace=1&completed=2015-03-08%2019:05:02" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=3&lane=2&firstname=Grayson&lastname=Shealey&carnumber=403&finishtime=3.3&finishplace=2&completed=2015-03-08%2019:05:02" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=3&lane=3&firstname=Gary&lastname=Rayes&carnumber=408&finishtime=3.648&finishplace=4&completed=2015-03-08%2019:05:02" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=3&lane=4&firstname=Alexander&lastname=Lisle&carnumber=406&finishtime=3.342&finishplace=3&completed=2015-03-08%2019:05:02" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=4&lane=1&firstname=Zachary&lastname=Strawser&carnumber=402&finishtime=3.143&finishplace=1&completed=2015-03-08%2019:06:29" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=4&lane=2&firstname=Henry&lastname=Henley&carnumber=405&finishtime=3.375&finishplace=4&completed=2015-03-08%2019:06:29" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=4&lane=3&firstname=Frank&lastname=Mutch&carnumber=404&finishtime=3.29&finishplace=3&completed=2015-03-08%2019:06:29" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=4&lane=4&firstname=Benjamin&lastname=Lisle&carnumber=407&finishtime=3.286&finishplace=2&completed=2015-03-08%2019:06:29" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=5&lane=1&firstname=Grayson&lastname=Shealey&carnumber=403&finishtime=3.233&finishplace=2&completed=2015-03-08%2019:08:00" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=5&lane=2&firstname=Gary&lastname=Rayes&carnumber=408&finishtime=3.708&finishplace=4&completed=2015-03-08%2019:08:00" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=5&lane=3&firstname=Evan&lastname=Largo&carnumber=401&finishtime=3.508&finishplace=3&completed=2015-03-08%2019:08:00" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=5&lane=4&firstname=Zachary&lastname=Strawser&carnumber=402&finishtime=3.159&finishplace=1&completed=2015-03-08%2019:08:00" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=6&lane=1&firstname=Henry&lastname=Henley&carnumber=405&finishtime=3.348&finishplace=3&completed=2015-03-08%2019:09:30" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=6&lane=2&firstname=Frank&lastname=Mutch&carnumber=404&finishtime=3.322&finishplace=2&completed=2015-03-08%2019:09:30" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=6&lane=3&firstname=Gaurav&lastname=Gros&carnumber=409&finishtime=3.238&finishplace=1&completed=2015-03-08%2019:09:30" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=6&lane=4&firstname=Grayson&lastname=Shealey&carnumber=403&finishtime=8.743&finishplace=4&completed=2015-03-08%2019:09:30" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=7&lane=1&firstname=Gary&lastname=Rayes&carnumber=408&finishtime=3.81&finishplace=4&completed=2015-03-08%2019:12:54" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=7&lane=2&firstname=Evan&lastname=Largo&carnumber=401&finishtime=3.515&finishplace=3&completed=2015-03-08%2019:12:54" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=7&lane=3&firstname=Alexander&lastname=Lisle&carnumber=406&finishtime=3.29&finishplace=1&completed=2015-03-08%2019:12:54" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=7&lane=4&firstname=Henry&lastname=Henley&carnumber=405&finishtime=3.427&finishplace=2&completed=2015-03-08%2019:12:54" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=8&lane=1&firstname=Frank&lastname=Mutch&carnumber=404&finishtime=3.278&finishplace=3&completed=2015-03-08%2019:14:08" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=8&lane=2&firstname=Gaurav&lastname=Gros&carnumber=409&finishtime=3.21&finishplace=2&completed=2015-03-08%2019:14:08" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=8&lane=3&firstname=Benjamin&lastname=Lisle&carnumber=407&finishtime=3.188&finishplace=1&completed=2015-03-08%2019:14:08" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=8&lane=4&firstname=Gary&lastname=Rayes&carnumber=408&finishtime=3.845&finishplace=4&completed=2015-03-08%2019:14:08" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=9&lane=1&firstname=Evan&lastname=Largo&carnumber=401&finishtime=3.521&finishplace=4&completed=2015-03-08%2019:15:34" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=9&lane=2&firstname=Alexander&lastname=Lisle&carnumber=406&finishtime=3.309&finishplace=3&completed=2015-03-08%2019:15:34" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=9&lane=3&firstname=Zachary&lastname=Strawser&carnumber=402&finishtime=3.158&finishplace=1&completed=2015-03-08%2019:15:34" | check_success
curl_post action.php "action=import-one-result&class=Webelos&round=1&heat=9&lane=4&firstname=Frank&lastname=Mutch&carnumber=404&finishtime=3.301&finishplace=2&completed=2015-03-08%2019:15:34" | check_success

curl_post action.php "action=import-one-result&class=Pace%20Car&round=1&heat=1&lane=1&firstname=Bruce&lastname=Henley&carnumber=501&finishtime=3.425&finishplace=1&completed=2015-03-08%2019:19:12" | check_success
curl_post action.php "action=import-one-result&class=Pace%20Car&round=1&heat=1&lane=4&firstname=Jeff&lastname=Piazza&carnumber=502&finishtime=3.775&finishplace=2&completed=2015-03-08%2019:19:12" | check_success
curl_post action.php "action=import-one-result&class=Pace%20Car&round=1&heat=2&lane=1&firstname=Jeff&lastname=Piazza&carnumber=502&finishtime=3.751&finishplace=2&completed=2015-03-08%2019:19:48" | check_success
curl_post action.php "action=import-one-result&class=Pace%20Car&round=1&heat=2&lane=2&firstname=Bruce&lastname=Henley&carnumber=501&finishtime=3.353&finishplace=1&completed=2015-03-08%2019:19:48" | check_success
curl_post action.php "action=import-one-result&class=Pace%20Car&round=1&heat=3&lane=2&firstname=Jeff&lastname=Piazza&carnumber=502&finishtime=3.705&finishplace=2&completed=2015-03-08%2019:20:25" | check_success
curl_post action.php "action=import-one-result&class=Pace%20Car&round=1&heat=3&lane=3&firstname=Bruce&lastname=Henley&carnumber=501&finishtime=3.366&finishplace=1&completed=2015-03-08%2019:20:25" | check_success
curl_post action.php "action=import-one-result&class=Pace%20Car&round=1&heat=4&lane=3&firstname=Jeff&lastname=Piazza&carnumber=502&finishtime=9.999&finishplace=2&completed=2015-03-08%2019:21:34" | check_success
curl_post action.php "action=import-one-result&class=Pace%20Car&round=1&heat=4&lane=4&firstname=Bruce&lastname=Henley&carnumber=501&finishtime=9.999&finishplace=1&completed=2015-03-08%2019:21:34" | check_success
