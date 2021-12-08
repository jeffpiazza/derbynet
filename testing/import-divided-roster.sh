#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# Data produced by: http://listofrandomnames.com

curl_postj action.php "action=racer.import&firstname=Lucius&lastname=Lemon" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Hubert&lastname=Heffley" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Tangela&lastname=Tacey&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Audry&lastname=Appleby&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kip&lastname=Kennelly&partition=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Charlena&lastname=Callas" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sherley&lastname=Summerford" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Roland&lastname=Rivero&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Gerald&lastname=Glennon&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Shalanda&lastname=Sanluis&partition=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Nohemi&lastname=Natale" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lien&lastname=Licht" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rhett&lastname=Roeder&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ivey&lastname=Ivester&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Christy&lastname=Capo&partition=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Dara&lastname=Dittmer" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Iraida&lastname=Icenhour" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Shirley&lastname=Savidge&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Daphine&lastname=Dimas&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Bonny&lastname=Barrow&partition=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Natisha&lastname=Neifert" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jesse&lastname=John" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lillia&lastname=Lawson&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Anh&lastname=Antonio&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Annalee&lastname=Aguila&partition=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Neomi&lastname=Newlin" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lyle&lastname=Lahey" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Denny&lastname=Daggett&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rosemarie&lastname=Robidoux&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Margot&lastname=Mckeithan&partition=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ruthe&lastname=Rooks" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sherell&lastname=Strode" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Demetrice&lastname=Denmark&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lakita&lastname=Lanman&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Iva&lastname=Imes&partition=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kellie&lastname=Kubacki" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Amparo&lastname=Ashbrook" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Maryetta&lastname=Menchaca&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Edwardo&lastname=Estabrook&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Librada&lastname=Labat&partition=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Vita&lastname=Villeda" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Karrie&lastname=Koski" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Charleen&lastname=Crist&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rosenda&lastname=Rossiter&partition=Div%202" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Isobel&lastname=Ivester&partition=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Nelia&lastname=Nagao" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Tuan&lastname=Troxell" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lynelle&lastname=Lipsey&partition=Div%202" | check_jsuccess
## curl_postj action.php "action=racer.import&firstname=Mose&lastname=Meisinger&partition=Div%202" | check_jsuccess
## Last racer, in Div 3, is racerid 49
curl_postj action.php "action=racer.import&firstname=Melanie&lastname=Meacham&partition=Div%203" | check_jsuccess


curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 3 and 
                .[0].name == "Default" and 
                .[0].count == 20 and
                .[1].name == "Div 2" and
                .[1].count == 19 and
                .[2].name == "Div 3" and
                .[2].count == 10' >/dev/null || test_fails
