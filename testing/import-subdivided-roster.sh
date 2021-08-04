#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# Data produced by: http://listofrandomnames.com

# 3 divisions, with 2, 2, and 1 subdivision, respectively

curl_postj action.php "action=racer.import&firstname=Lucius&lastname=Lemon&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Hubert&lastname=Heffley&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Tangela&lastname=Tacey&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Audry&lastname=Appleby&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kip&lastname=Kennelly&division=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Charlena&lastname=Callas&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sherley&lastname=Summerford&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Roland&lastname=Rivero&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Gerald&lastname=Glennon&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Shalanda&lastname=Sanluis&division=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Nohemi&lastname=Natale&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lien&lastname=Licht&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rhett&lastname=Roeder&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ivey&lastname=Ivester&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Christy&lastname=Capo&division=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Dara&lastname=Dittmer&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Iraida&lastname=Icenhour&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Shirley&lastname=Savidge&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Daphine&lastname=Dimas&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Bonny&lastname=Barrow&division=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Natisha&lastname=Neifert&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jesse&lastname=John&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lillia&lastname=Lawson&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Anh&lastname=Antonio&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Annalee&lastname=Aguila&division=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Neomi&lastname=Newlin&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lyle&lastname=Lahey&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Denny&lastname=Daggett&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rosemarie&lastname=Robidoux&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Margot&lastname=Mckeithan&division=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ruthe&lastname=Rooks&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sherell&lastname=Strode&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Demetrice&lastname=Denmark&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lakita&lastname=Lanman&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Iva&lastname=Imes&division=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Kellie&lastname=Kubacki&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Amparo&lastname=Ashbrook&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Maryetta&lastname=Menchaca&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Edwardo&lastname=Estabrook&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Librada&lastname=Labat&division=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Vita&lastname=Villeda&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Karrie&lastname=Koski&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Charleen&lastname=Crist&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Rosenda&lastname=Rossiter&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Isobel&lastname=Ivester&division=Div%203" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Nelia&lastname=Nagao&subdivision=Subdiv%201a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Tuan&lastname=Troxell&subdivision=Subdiv%201b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Lynelle&lastname=Lipsey&division=Div%202&subdivision=Subdiv%202b" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Mose&lastname=Meisinger&division=Div%202&subdivision=Subdiv%202a" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Melanie&lastname=Meacham&division=Div%203" | check_jsuccess


curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3 and 
                .[0].name == "Default" and 
                (.[0].subdivisions | length == 2) and
                .[1].name == "Div 2" and
                (.[1].subdivisions | length == 2) and
                .[2].name == "Div 3" and
                (.[2].subdivisions | length == 1)' >/dev/null || test_fails
