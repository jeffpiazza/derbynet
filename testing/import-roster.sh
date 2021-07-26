#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# Data produced by: http://listofrandomnames.com

# racerid's are assigned in order, starting with 1 (assuming an empty database).
# The car numbers in this data set are constructed so that car number mod 100 gives the racerid.

curl_postj action.php "action=racer.import&firstname=Adolfo \"Dolf\"&lastname=Asher&division=Lions %26 Tigers&carnumber=101" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Angelo&lastname=Alas&division=White's Wolves&carnumber=202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Antoine&lastname=Akiyama&division=Bears%20and%20Fr%C3%A8res&carnumber=303" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Arden&lastname=Aziz&division=Webelos (\"Webes\")&carnumber=504" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Barney&lastname=Bainter&division=Arrows %3C%3C--%3C%3C&carnumber=405" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ben&lastname=Bittinger&division=Lions %26 Tigers&carnumber=106" | check_jsuccess
# curl_postj action.php "action=racer.import&firstname=Blake&lastname=Burling&division=White's Wolves&carnumber=207" | check_jsuccess
curl_postj action.php "action=racer.import&first-last=Blake Burling&division=White's Wolves&carnumber=207" | check_jsuccess
# curl_postj action.php "action=racer.import&firstname=Bruce&lastname=Boissonneault&division=Bears%20and%20Fr%C3%A8res&carnumber=308" | check_jsuccess
curl_postj action.php "action=racer.import&first-last= Bruce  Boissonneault &division=Bears%20and%20Fr%C3%A8res&carnumber=308" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Byron&lastname=Billy&division=Webelos (\"Webes\")&carnumber=509" | check_jsuccess
# curl_postj action.php "action=racer.import&firstname=Carey&lastname=Craney&division=Arrows %3C%3C--%3C%3C&carnumber=410" | check_jsuccess
curl_postj action.php "action=racer.import&first-last=Craney, Carey&division=Arrows %3C%3C--%3C%3C&carnumber=410" | check_jsuccess
#curl_postj action.php "action=racer.import&firstname=Carroll&lastname=Cybulski&division=Lions %26 Tigers&carnumber=111" | check_jsuccess
curl_postj action.php "action=racer.import&first-last=Cybulski,Carroll&division=Lions %26 Tigers&carnumber=111" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Christoper&lastname=Chauncey&division=White's Wolves&carnumber=212" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Clark&lastname=Chesnutt&division=Bears%20and%20Fr%C3%A8res&carnumber=313" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Cletus&lastname=Creager&division=Webelos (\"Webes\")&carnumber=514" | check_jsuccess
# 
curl_postj action.php "action=racer.import&firstname=Ca%C3%B1umil&lastname=Calero&division=Arrows %3C%3C--%3C%3C&carnumber=415" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Danial&lastname=Depaolo&division=Lions %26 Tigers&carnumber=116" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Darrell %26 Darrell&lastname=Delaughter&division=White's Wolves&carnumber=217" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Darrin&lastname=Denny&division=Bears%20and%20Fr%C3%A8res&carnumber=318" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Denny&lastname=Deering&division=Webelos (\"Webes\")&carnumber=519" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Derek&lastname=Dantonio&division=Arrows %3C%3C--%3C%3C&carnumber=420" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Derick&lastname=Dreier&division=Lions %26 Tigers&carnumber=121" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Dexter&lastname=Dawes&division=White's Wolves&carnumber=222" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Domingo&lastname=Doles&division=Bears%20and%20Fr%C3%A8res&carnumber=323" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Dorian&lastname=Dunkle&division=Webelos (\"Webes\")&carnumber=524" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Earnest&lastname=Evangelista&division=Arrows %3C%3C--%3C%3C&carnumber=425" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Edgardo&lastname=Easterwood&division=Lions %26 Tigers&carnumber=126" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Elliot&lastname=Eastman&division=White's Wolves&carnumber=227" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ethan&lastname=Enye&division=Bears%20and%20Fr%C3%A8res&carnumber=328" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Emory&lastname=Ertel&division=Webelos (\"Webes\")&carnumber=529" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Enoch&lastname=Eccles&division=Arrows %3C%3C--%3C%3C&carnumber=430" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Felton&lastname=Fouche&division=Lions %26 Tigers&carnumber=131" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Freddie&lastname=Font&division=White's Wolves&carnumber=232" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Freeman&lastname=Fizer&division=Bears%20and%20Fr%C3%A8res&carnumber=333" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Gregg&lastname=Grove&division=Webelos (\"Webes\")&carnumber=534" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Harley&lastname=Howell&division=Arrows %3C%3C--%3C%3C&carnumber=435" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Herb&lastname=Halfacre&division=Lions %26 Tigers&carnumber=136" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ian&lastname=Ives&division=White's Wolves&carnumber=237" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jackson&lastname=Juliano&division=Bears%20and%20Fr%C3%A8res&carnumber=338" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jamison&lastname=Jeffress&division=Webelos (\"Webes\")&carnumber=539" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jed&lastname=Jaquez&division=Arrows %3C%3C--%3C%3C&carnumber=440" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jesse&lastname=Jara&division=Lions %26 Tigers&carnumber=141" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Josh&lastname=Jose&division=White's Wolves&carnumber=242" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Juan&lastname=Jacobsen&division=Bears%20and%20Fr%C3%A8res&carnumber=343" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Judson&lastname=Joynt&division=Webelos (\"Webes\")&carnumber=544" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Julian&lastname=Jarrard&division=Arrows %3C%3C--%3C%3C&carnumber=445" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kelvin&lastname=Kinman&division=Lions %26 Tigers&carnumber=146" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kelvin&lastname=Knapp&division=White's Wolves&carnumber=247" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kory&lastname=Kilgo&division=Bears%20and%20Fr%C3%A8res&carnumber=348" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kris&lastname=Kaba&division=Webelos (\"Webes\")&carnumber=549" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lanny&lastname=Lavigne&division=Arrows %3C%3C--%3C%3C&carnumber=450" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Levi&lastname=Lahr&division=Lions %26 Tigers&carnumber=151" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lewis&lastname=Levitsky&division=White's Wolves&carnumber=252" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lyman&lastname=Liller&division=Bears%20and%20Fr%C3%A8res&carnumber=353" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Markus&lastname=Muncy&division=Webelos (\"Webes\")&carnumber=554" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Marlon&lastname=McGray&division=Arrows %3C%3C--%3C%3C&carnumber=455" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Michal&lastname=Melendrez&division=Lions %26 Tigers&carnumber=156" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Mohamed&lastname=McGrew&division=White's Wolves&carnumber=257" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Nelson&lastname=No&division=Bears%20and%20Fr%C3%A8res&carnumber=358" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Norbert&lastname=Nightingale&division=Webelos (\"Webes\")&carnumber=559" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Numbers&lastname=Nish&division=Arrows %3C%3C--%3C%3C&carnumber=460" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Owen&lastname=O'Leary&division=Lions %26 Tigers&carnumber=161" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Pat&lastname=Petrone&division=White's Wolves&carnumber=262" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Pete&lastname=Pinkney&division=Bears%20and%20Fr%C3%A8res&carnumber=363" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Philip&lastname=Prum&division=Webelos (\"Webes\")&carnumber=564" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Porter&lastname=Papke&division=Arrows %3C%3C--%3C%3C&carnumber=465" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Raymon&lastname=Ruffner&division=Lions %26 Tigers&carnumber=166" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Renaldo&lastname=Raposo&division=White's Wolves&carnumber=267" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Reuben&lastname=Rockhill&division=Bears%20and%20Fr%C3%A8res&carnumber=368" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rex&lastname=Rosalez&division=Webelos (\"Webes\")&carnumber=569" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Robbie&lastname=Roush&division=Arrows %3C%3C--%3C%3C&carnumber=470" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rodrigo&lastname=Rencher&division=Lions %26 Tigers&carnumber=171" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Royce&lastname=Rohman&division=White's Wolves&carnumber=272" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Scottie&lastname=Servais&division=Bears%20and%20Fr%C3%A8res&carnumber=373" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sean&lastname=Strasburg&division=Webelos (\"Webes\")&carnumber=574" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Timmy&lastname=Tomei&division=Arrows %3C%3C--%3C%3C&carnumber=475" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Toby&lastname=Teed&division=Lions %26 Tigers&carnumber=176" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Tracey&lastname=Trapp&division=White's Wolves&carnumber=277" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Travis&lastname=Toothaker&division=Bears%20and%20Fr%C3%A8res&carnumber=378" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Vincent&lastname=Vinci&division=Webelos (\"Webes\")&carnumber=579" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Weston&lastname=Whigham&division=Arrows %3C%3C--%3C%3C&carnumber=480" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Willard&lastname=Wile&division=Lions %26 Tigers&carnumber=181" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Willard&lastname=Woolfolk&division=White's Wolves&carnumber=282" | check_jsuccess

# Import one last entry and then immediately delete
curl_postj action.php "action=racer.import&firstname=Soon&lastname=ToGo&division=White's Wolves&carnumber=283" | check_jsuccess

curl_getj "action.php?query=racer.list" | expect_one 'ToGo'
curl_postj action.php "action=racer.delete&racer=83" | check_jsuccess
curl_getj "action.php?query=racer.list" | expect_count 'ToGo' 0

curl_postj action.php "action=racer.bulk&what=number&who=c4&start=501&renumber=1" | check_jsuccess

