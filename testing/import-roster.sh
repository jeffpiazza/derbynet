#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# Data produced by: http://listofrandomnames.com

# racerid's are assigned in order, starting with 1 (assuming an empty database).
# The car numbers in this data set are constructed so that car number mod 100 gives the racerid.

curl_postj action.php "action=racer.import&firstname=Adolfo \"Dolf\"&lastname=Asher&partition=Lions %26 Tigers&carnumber=101&note_from=Anywhere" | check_jsuccess
curl_getj "action.php?query=racer.list" | \
    jq -e '.racers | map(select(.lastname=="Asher"))[0].note_from == "Anywhere"' > /dev/null || test_fails

curl_postj action.php "action=racer.import&firstname=Angelo&lastname=Alas&partition=White's Wolves&carnumber=202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Antoine&lastname=Akiyama&partition=Bears%20and%20Fr%C3%A8res&carnumber=303" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Arden&lastname=Aziz&partition=Webelos (\"Webes\")&carnumber=504" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Barney&lastname=Bainter&partition=Arrows %3C%3C--%3C%3C&carnumber=405" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ben&lastname=Bittinger&partition=Lions %26 Tigers&carnumber=106" | check_jsuccess
# curl_postj action.php "action=racer.import&firstname=Blake&lastname=Burling&partition=White's Wolves&carnumber=207" | check_jsuccess
curl_postj action.php "action=racer.import&first-last=Blake Burling&partition=White's Wolves&carnumber=207" | check_jsuccess
# curl_postj action.php "action=racer.import&firstname=Bruce&lastname=Boissonneault&partition=Bears%20and%20Fr%C3%A8res&carnumber=308" | check_jsuccess
curl_postj action.php "action=racer.import&first-last= Bruce  Boissonneault &partition=Bears%20and%20Fr%C3%A8res&carnumber=308" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Byron&lastname=Billy&partition=Webelos (\"Webes\")&carnumber=509" | check_jsuccess
# curl_postj action.php "action=racer.import&firstname=Carey&lastname=Craney&partition=Arrows %3C%3C--%3C%3C&carnumber=410" | check_jsuccess
curl_postj action.php "action=racer.import&first-last=Craney, Carey&partition=Arrows %3C%3C--%3C%3C&carnumber=410" | check_jsuccess
#curl_postj action.php "action=racer.import&firstname=Carroll&lastname=Cybulski&partition=Lions %26 Tigers&carnumber=111" | check_jsuccess
curl_postj action.php "action=racer.import&first-last=Cybulski,Carroll&partition=Lions %26 Tigers&carnumber=111" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Christoper&lastname=Chauncey&partition=White's Wolves&carnumber=212" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Clark&lastname=Chesnutt&partition=Bears%20and%20Fr%C3%A8res&carnumber=313" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Cletus&lastname=Creager&partition=Webelos (\"Webes\")&carnumber=514" | check_jsuccess
# 
curl_postj action.php "action=racer.import&firstname=Ca%C3%B1umil&lastname=Calero&partition=Arrows %3C%3C--%3C%3C&carnumber=415" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Danial&lastname=Depaolo&partition=Lions %26 Tigers&carnumber=116" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Darrell %26 Darrell&lastname=Delaughter&partition=White's Wolves&carnumber=217" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Darrin&lastname=Denny&partition=Bears%20and%20Fr%C3%A8res&carnumber=318" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Denny&lastname=Deering&partition=Webelos (\"Webes\")&carnumber=519" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Derek&lastname=Dantonio&partition=Arrows %3C%3C--%3C%3C&carnumber=420" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Derick&lastname=Dreier&partition=Lions %26 Tigers&carnumber=121" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Dexter&lastname=Dawes&partition=White's Wolves&carnumber=222" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Domingo&lastname=Doles&partition=Bears%20and%20Fr%C3%A8res&carnumber=323" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Dorian&lastname=Dunkle&partition=Webelos (\"Webes\")&carnumber=524" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Earnest&lastname=Evangelista&partition=Arrows %3C%3C--%3C%3C&carnumber=425" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Edgardo&lastname=Easterwood&partition=Lions %26 Tigers&carnumber=126" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Elliot&lastname=Eastman&partition=White's Wolves&carnumber=227" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ethan&lastname=Enye&partition=Bears%20and%20Fr%C3%A8res&carnumber=328" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Emory&lastname=Ertel&partition=Webelos (\"Webes\")&carnumber=529" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Enoch&lastname=Eccles&partition=Arrows %3C%3C--%3C%3C&carnumber=430" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Felton&lastname=Fouche&partition=Lions %26 Tigers&carnumber=131" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Freddie&lastname=Font&partition=White's Wolves&carnumber=232" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Freeman&lastname=Fizer&partition=Bears%20and%20Fr%C3%A8res&carnumber=333" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Gregg&lastname=Grove&partition=Webelos (\"Webes\")&carnumber=534" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Harley&lastname=Howell&partition=Arrows %3C%3C--%3C%3C&carnumber=435" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Herb&lastname=Halfacre&partition=Lions %26 Tigers&carnumber=136" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ian&lastname=Ives&partition=White's Wolves&carnumber=237" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jackson&lastname=Juliano&partition=Bears%20and%20Fr%C3%A8res&carnumber=338" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jamison&lastname=Jeffress&partition=Webelos (\"Webes\")&carnumber=539" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jed&lastname=Jaquez&partition=Arrows %3C%3C--%3C%3C&carnumber=440" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jesse&lastname=Jara&partition=Lions %26 Tigers&carnumber=141" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Josh&lastname=Jose&partition=White's Wolves&carnumber=242" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Juan&lastname=Jacobsen&partition=Bears%20and%20Fr%C3%A8res&carnumber=343" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Judson&lastname=Joynt&partition=Webelos (\"Webes\")&carnumber=544" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Julian&lastname=Jarrard&partition=Arrows %3C%3C--%3C%3C&carnumber=445" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kelvin&lastname=Kinman&partition=Lions %26 Tigers&carnumber=146" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kelvin&lastname=Knapp&partition=White's Wolves&carnumber=247" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kory&lastname=Kilgo&partition=Bears%20and%20Fr%C3%A8res&carnumber=348" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kris&lastname=Kaba&partition=Webelos (\"Webes\")&carnumber=549" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lanny&lastname=Lavigne&partition=Arrows %3C%3C--%3C%3C&carnumber=450" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Levi&lastname=Lahr&partition=Lions %26 Tigers&carnumber=151" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lewis&lastname=Levitsky&partition=White's Wolves&carnumber=252" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lyman&lastname=Liller&partition=Bears%20and%20Fr%C3%A8res&carnumber=353" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Markus&lastname=Muncy&partition=Webelos (\"Webes\")&carnumber=554" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Marlon&lastname=McGray&partition=Arrows %3C%3C--%3C%3C&carnumber=455" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Michal&lastname=Melendrez&partition=Lions %26 Tigers&carnumber=156" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Mohamed&lastname=McGrew&partition=White's Wolves&carnumber=257" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Nelson&lastname=No&partition=Bears%20and%20Fr%C3%A8res&carnumber=358" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Norbert&lastname=Nightingale&partition=Webelos (\"Webes\")&carnumber=559" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Numbers&lastname=Nish&partition=Arrows %3C%3C--%3C%3C&carnumber=460" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Owen&lastname=O'Leary&partition=Lions %26 Tigers&carnumber=161" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Pat&lastname=Petrone&partition=White's Wolves&carnumber=262" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Pete&lastname=Pinkney&partition=Bears%20and%20Fr%C3%A8res&carnumber=363" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Philip&lastname=Prum&partition=Webelos (\"Webes\")&carnumber=564" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Porter&lastname=Papke&partition=Arrows %3C%3C--%3C%3C&carnumber=465" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Raymon&lastname=Ruffner&partition=Lions %26 Tigers&carnumber=166" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Renaldo&lastname=Raposo&partition=White's Wolves&carnumber=267" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Reuben&lastname=Rockhill&partition=Bears%20and%20Fr%C3%A8res&carnumber=368" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rex&lastname=Rosalez&partition=Webelos (\"Webes\")&carnumber=569" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Robbie&lastname=Roush&partition=Arrows %3C%3C--%3C%3C&carnumber=470" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rodrigo&lastname=Rencher&partition=Lions %26 Tigers&carnumber=171" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Royce&lastname=Rohman&partition=White's Wolves&carnumber=272" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Scottie&lastname=Servais&partition=Bears%20and%20Fr%C3%A8res&carnumber=373" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sean&lastname=Strasburg&partition=Webelos (\"Webes\")&carnumber=574" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Timmy&lastname=Tomei&partition=Arrows %3C%3C--%3C%3C&carnumber=475" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Toby&lastname=Teed&partition=Lions %26 Tigers&carnumber=176" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Tracey&lastname=Trapp&partition=White's Wolves&carnumber=277" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Travis&lastname=Toothaker&partition=Bears%20and%20Fr%C3%A8res&carnumber=378" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Vincent&lastname=Vinci&partition=Webelos (\"Webes\")&carnumber=579" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Weston&lastname=Whigham&partition=Arrows %3C%3C--%3C%3C&carnumber=480" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Willard&lastname=Wile&partition=Lions %26 Tigers&carnumber=181" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Willard&lastname=Woolfolk&partition=White's Wolves&carnumber=282" | check_jsuccess

# Import one last entry and then immediately delete
curl_postj action.php "action=racer.import&firstname=Soon&lastname=ToGo&partition=White's Wolves&carnumber=283" | check_jsuccess

curl_getj "action.php?query=racer.list" | expect_one 'ToGo'

TOGO_RACERID=$(jq  '.racers | map(select(.lastname == "ToGo"))[0].racerid'  testing/debug.curl)

curl_postj action.php "action=racer.delete&racer=$TOGO_RACERID" | check_jsuccess
curl_getj "action.php?query=racer.list" | expect_count 'ToGo' 0

curl_postj action.php "action=racer.bulk&what=number&who=c4&start=501&renumber=1" | check_jsuccess

