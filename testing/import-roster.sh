#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# Data produced by: http://listofrandomnames.com

curl_post action.php "action=racer.import&firstname=Adolfo \"Dolf\"&lastname=Asher&classname=Lions %26 Tigers&carnumber=101" | check_success
curl_post action.php "action=racer.import&firstname=Angelo&lastname=Alas&classname=White's Wolves&carnumber=202" | check_success
curl_post action.php "action=racer.import&firstname=Antoine&lastname=Akiyama&classname=Bears%20and%20Fr%C3%A8res&carnumber=303" | check_success
curl_post action.php "action=racer.import&firstname=Arden&lastname=Aziz&classname=Webelos (\"Webes\")&carnumber=504" | check_success
curl_post action.php "action=racer.import&firstname=Barney&lastname=Bainter&classname=Arrows %3C%3C--%3C%3C&carnumber=405" | check_success
curl_post action.php "action=racer.import&firstname=Ben&lastname=Bittinger&classname=Lions %26 Tigers&carnumber=106" | check_success
# curl_post action.php "action=racer.import&firstname=Blake&lastname=Burling&classname=White's Wolves&carnumber=207" | check_success
curl_post action.php "action=racer.import&first-last=Blake Burling&classname=White's Wolves&carnumber=207" | check_success
# curl_post action.php "action=racer.import&firstname=Bruce&lastname=Boissonneault&classname=Bears%20and%20Fr%C3%A8res&carnumber=308" | check_success
curl_post action.php "action=racer.import&first-last= Bruce  Boissonneault &classname=Bears%20and%20Fr%C3%A8res&carnumber=308" | check_success
curl_post action.php "action=racer.import&firstname=Byron&lastname=Billy&classname=Webelos (\"Webes\")&carnumber=509" | check_success
# curl_post action.php "action=racer.import&firstname=Carey&lastname=Craney&classname=Arrows %3C%3C--%3C%3C&carnumber=410" | check_success
curl_post action.php "action=racer.import&first-last=Craney, Carey&classname=Arrows %3C%3C--%3C%3C&carnumber=410" | check_success
#curl_post action.php "action=racer.import&firstname=Carroll&lastname=Cybulski&classname=Lions %26 Tigers&carnumber=111" | check_success
curl_post action.php "action=racer.import&first-last=Cybulski,Carroll&classname=Lions %26 Tigers&carnumber=111" | check_success
curl_post action.php "action=racer.import&firstname=Christoper&lastname=Chauncey&classname=White's Wolves&carnumber=212" | check_success
curl_post action.php "action=racer.import&firstname=Clark&lastname=Chesnutt&classname=Bears%20and%20Fr%C3%A8res&carnumber=313" | check_success
curl_post action.php "action=racer.import&firstname=Cletus&lastname=Creager&classname=Webelos (\"Webes\")&carnumber=514" | check_success
# 
curl_post action.php "action=racer.import&firstname=Ca%C3%B1umil&lastname=Calero&classname=Arrows %3C%3C--%3C%3C&carnumber=415" | check_success
curl_post action.php "action=racer.import&firstname=Danial&lastname=Depaolo&classname=Lions %26 Tigers&carnumber=116" | check_success
curl_post action.php "action=racer.import&firstname=Darrell %26 Darrell&lastname=Delaughter&classname=White's Wolves&carnumber=217" | check_success
curl_post action.php "action=racer.import&firstname=Darrin&lastname=Denny&classname=Bears%20and%20Fr%C3%A8res&carnumber=318" | check_success
curl_post action.php "action=racer.import&firstname=Denny&lastname=Deering&classname=Webelos (\"Webes\")&carnumber=519" | check_success
curl_post action.php "action=racer.import&firstname=Derek&lastname=Dantonio&classname=Arrows %3C%3C--%3C%3C&carnumber=420" | check_success
curl_post action.php "action=racer.import&firstname=Derick&lastname=Dreier&classname=Lions %26 Tigers&carnumber=121" | check_success
curl_post action.php "action=racer.import&firstname=Dexter&lastname=Dawes&classname=White's Wolves&carnumber=222" | check_success
curl_post action.php "action=racer.import&firstname=Domingo&lastname=Doles&classname=Bears%20and%20Fr%C3%A8res&carnumber=323" | check_success
curl_post action.php "action=racer.import&firstname=Dorian&lastname=Dunkle&classname=Webelos (\"Webes\")&carnumber=524" | check_success
curl_post action.php "action=racer.import&firstname=Earnest&lastname=Evangelista&classname=Arrows %3C%3C--%3C%3C&carnumber=425" | check_success
curl_post action.php "action=racer.import&firstname=Edgardo&lastname=Easterwood&classname=Lions %26 Tigers&carnumber=126" | check_success
curl_post action.php "action=racer.import&firstname=Elliot&lastname=Eastman&classname=White's Wolves&carnumber=227" | check_success
curl_post action.php "action=racer.import&firstname=Ethan&lastname=Enye&classname=Bears%20and%20Fr%C3%A8res&carnumber=328" | check_success
curl_post action.php "action=racer.import&firstname=Emory&lastname=Ertel&classname=Webelos (\"Webes\")&carnumber=529" | check_success
curl_post action.php "action=racer.import&firstname=Enoch&lastname=Eccles&classname=Arrows %3C%3C--%3C%3C&carnumber=430" | check_success
curl_post action.php "action=racer.import&firstname=Felton&lastname=Fouche&classname=Lions %26 Tigers&carnumber=131" | check_success
curl_post action.php "action=racer.import&firstname=Freddie&lastname=Font&classname=White's Wolves&carnumber=232" | check_success
curl_post action.php "action=racer.import&firstname=Freeman&lastname=Fizer&classname=Bears%20and%20Fr%C3%A8res&carnumber=333" | check_success
curl_post action.php "action=racer.import&firstname=Gregg&lastname=Grove&classname=Webelos (\"Webes\")&carnumber=534" | check_success
curl_post action.php "action=racer.import&firstname=Harley&lastname=Howell&classname=Arrows %3C%3C--%3C%3C&carnumber=435" | check_success
curl_post action.php "action=racer.import&firstname=Herb&lastname=Halfacre&classname=Lions %26 Tigers&carnumber=136" | check_success
curl_post action.php "action=racer.import&firstname=Ian&lastname=Ives&classname=White's Wolves&carnumber=237" | check_success
curl_post action.php "action=racer.import&firstname=Jackson&lastname=Juliano&classname=Bears%20and%20Fr%C3%A8res&carnumber=338" | check_success
curl_post action.php "action=racer.import&firstname=Jamison&lastname=Jeffress&classname=Webelos (\"Webes\")&carnumber=539" | check_success
curl_post action.php "action=racer.import&firstname=Jed&lastname=Jaquez&classname=Arrows %3C%3C--%3C%3C&carnumber=440" | check_success
curl_post action.php "action=racer.import&firstname=Jesse&lastname=Jara&classname=Lions %26 Tigers&carnumber=141" | check_success
curl_post action.php "action=racer.import&firstname=Josh&lastname=Jose&classname=White's Wolves&carnumber=242" | check_success
curl_post action.php "action=racer.import&firstname=Juan&lastname=Jacobsen&classname=Bears%20and%20Fr%C3%A8res&carnumber=343" | check_success
curl_post action.php "action=racer.import&firstname=Judson&lastname=Joynt&classname=Webelos (\"Webes\")&carnumber=544" | check_success
curl_post action.php "action=racer.import&firstname=Julian&lastname=Jarrard&classname=Arrows %3C%3C--%3C%3C&carnumber=445" | check_success
curl_post action.php "action=racer.import&firstname=Kelvin&lastname=Kinman&classname=Lions %26 Tigers&carnumber=146" | check_success
curl_post action.php "action=racer.import&firstname=Kelvin&lastname=Knapp&classname=White's Wolves&carnumber=247" | check_success
curl_post action.php "action=racer.import&firstname=Kory&lastname=Kilgo&classname=Bears%20and%20Fr%C3%A8res&carnumber=348" | check_success
curl_post action.php "action=racer.import&firstname=Kris&lastname=Kaba&classname=Webelos (\"Webes\")&carnumber=549" | check_success
curl_post action.php "action=racer.import&firstname=Lanny&lastname=Lavigne&classname=Arrows %3C%3C--%3C%3C&carnumber=450" | check_success
curl_post action.php "action=racer.import&firstname=Levi&lastname=Lahr&classname=Lions %26 Tigers&carnumber=151" | check_success
curl_post action.php "action=racer.import&firstname=Lewis&lastname=Levitsky&classname=White's Wolves&carnumber=252" | check_success
curl_post action.php "action=racer.import&firstname=Lyman&lastname=Liller&classname=Bears%20and%20Fr%C3%A8res&carnumber=353" | check_success
curl_post action.php "action=racer.import&firstname=Markus&lastname=Muncy&classname=Webelos (\"Webes\")&carnumber=554" | check_success
curl_post action.php "action=racer.import&firstname=Marlon&lastname=McGray&classname=Arrows %3C%3C--%3C%3C&carnumber=455" | check_success
curl_post action.php "action=racer.import&firstname=Michal&lastname=Melendrez&classname=Lions %26 Tigers&carnumber=156" | check_success
curl_post action.php "action=racer.import&firstname=Mohamed&lastname=McGrew&classname=White's Wolves&carnumber=257" | check_success
curl_post action.php "action=racer.import&firstname=Nelson&lastname=No&classname=Bears%20and%20Fr%C3%A8res&carnumber=358" | check_success
curl_post action.php "action=racer.import&firstname=Norbert&lastname=Nightingale&classname=Webelos (\"Webes\")&carnumber=559" | check_success
curl_post action.php "action=racer.import&firstname=Numbers&lastname=Nish&classname=Arrows %3C%3C--%3C%3C&carnumber=460" | check_success
curl_post action.php "action=racer.import&firstname=Owen&lastname=O'Leary&classname=Lions %26 Tigers&carnumber=161" | check_success
curl_post action.php "action=racer.import&firstname=Pat&lastname=Petrone&classname=White's Wolves&carnumber=262" | check_success
curl_post action.php "action=racer.import&firstname=Pete&lastname=Pinkney&classname=Bears%20and%20Fr%C3%A8res&carnumber=363" | check_success
curl_post action.php "action=racer.import&firstname=Philip&lastname=Prum&classname=Webelos (\"Webes\")&carnumber=564" | check_success
curl_post action.php "action=racer.import&firstname=Porter&lastname=Papke&classname=Arrows %3C%3C--%3C%3C&carnumber=465" | check_success
curl_post action.php "action=racer.import&firstname=Raymon&lastname=Ruffner&classname=Lions %26 Tigers&carnumber=166" | check_success
curl_post action.php "action=racer.import&firstname=Renaldo&lastname=Raposo&classname=White's Wolves&carnumber=267" | check_success
curl_post action.php "action=racer.import&firstname=Reuben&lastname=Rockhill&classname=Bears%20and%20Fr%C3%A8res&carnumber=368" | check_success
curl_post action.php "action=racer.import&firstname=Rex&lastname=Rosalez&classname=Webelos (\"Webes\")&carnumber=569" | check_success
curl_post action.php "action=racer.import&firstname=Robbie&lastname=Roush&classname=Arrows %3C%3C--%3C%3C&carnumber=470" | check_success
curl_post action.php "action=racer.import&firstname=Rodrigo&lastname=Rencher&classname=Lions %26 Tigers&carnumber=171" | check_success
curl_post action.php "action=racer.import&firstname=Royce&lastname=Rohman&classname=White's Wolves&carnumber=272" | check_success
curl_post action.php "action=racer.import&firstname=Scottie&lastname=Servais&classname=Bears%20and%20Fr%C3%A8res&carnumber=373" | check_success
curl_post action.php "action=racer.import&firstname=Sean&lastname=Strasburg&classname=Webelos (\"Webes\")&carnumber=574" | check_success
curl_post action.php "action=racer.import&firstname=Timmy&lastname=Tomei&classname=Arrows %3C%3C--%3C%3C&carnumber=475" | check_success
curl_post action.php "action=racer.import&firstname=Toby&lastname=Teed&classname=Lions %26 Tigers&carnumber=176" | check_success
curl_post action.php "action=racer.import&firstname=Tracey&lastname=Trapp&classname=White's Wolves&carnumber=277" | check_success
curl_post action.php "action=racer.import&firstname=Travis&lastname=Toothaker&classname=Bears%20and%20Fr%C3%A8res&carnumber=378" | check_success
curl_post action.php "action=racer.import&firstname=Vincent&lastname=Vinci&classname=Webelos (\"Webes\")&carnumber=579" | check_success
curl_post action.php "action=racer.import&firstname=Weston&lastname=Whigham&classname=Arrows %3C%3C--%3C%3C&carnumber=480" | check_success
curl_post action.php "action=racer.import&firstname=Willard&lastname=Wile&classname=Lions %26 Tigers&carnumber=181" | check_success
curl_post action.php "action=racer.import&firstname=Willard&lastname=Woolfolk&classname=White's Wolves&carnumber=282" | check_success

