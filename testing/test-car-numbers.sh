#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# user_login_coordinator

RESET_SOURCE=car-numbers `dirname $0`/reset-database.sh "$BASE_URL"


# $1 first-last
# $2 partition
# $3 expected car number
function expect_car_number() {
  RACERID=$(curl_postj action.php "action=racer.import&first-last=$1&partition=$2&exclude=" | \
                jq -e .racerid)
  curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.racerid==$RACERID))[0].carnumber" | expect_eq $3
}

N=101
for NAME in 'Gaige Nichols' \
             'Tiana Padilla' \
             'Heather Calderon' \
             'Dashawn Frederick' \
             'Owen Richardson' \
             'Madelyn Randall' \
             'Jaydin Bullock' \
             'Perla Marsh' \
             'Lily Braun' \
             'Liana Blair' \
             'Londyn Valencia' \
             'Alayna McGee' ; do
    expect_car_number "$NAME" Wolves $N
    N=$((N+1))
done

N=201
for NAME in 'Aliyah Lyons' \
                'Cameron Chaney' \
                'Layla Underwood' \
                'Abdullah Lawrence' \
                'Camille House' \
                'Alvaro Mclean' \
                'Kylie Gonzales' \
                'Derrick Lindsey' \
                'Danna Shepherd' \
                'Dakota Frey' \
                'Sergio Ray' ; do
    expect_car_number "$NAME" Tigers $N
    N=$((N+1))
done

N=301
for NAME in 'Rebecca Griffith' \
                'Raquel Vasquez' \
                'Kiera Wall' \
                'Shaun Baird' \
                'Jadyn Silva' \
                'Mckenna Olson' \
                'Dustin Valencia' \
                'Chanel Clarke' \
                'Trinity Lang' \
                'Ben Bryan' \
                'Valerie Peterson' ; do
    expect_car_number "$NAME" Lions $N
    N=$((N+1))
done


curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Gaige\"))[0].carnumber" | expect_eq 101
curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Alayna\"))[0].carnumber" | expect_eq 112

curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Aliyah\"))[0].carnumber" | expect_eq 201
curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Sergio\"))[0].carnumber" | expect_eq 211

curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Rebecca\"))[0].carnumber" | expect_eq 301
curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Valerie\"))[0].carnumber" | expect_eq 311

# Partitionid 1: Wolves
# Partitionid 2: Tigers
# Partitionid 3: Lions
# Re-order to Lions, Tigers, Wolves
curl_postj action.php "action=partition.order&partitionid_1=3&partitionid_2=2&partitionid_3=1" | check_jsuccess

curl_postj action.php "action=racer.bulk&what=number&who=all&renumber=1&auto=1" | check_jsuccess


curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Gaige\"))[0].carnumber" | expect_eq 301
curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Alayna\"))[0].carnumber" | expect_eq 312

curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Aliyah\"))[0].carnumber" | expect_eq 201
curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Sergio\"))[0].carnumber" | expect_eq 211

curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Rebecca\"))[0].carnumber" | expect_eq 101
curl_getj "action.php?query=racer.list" | \
      jq -e ".racers | map(select(.firstname==\"Valerie\"))[0].carnumber" | expect_eq 111


RESET_SOURCE=car-numbers-2 `dirname $0`/reset-database.sh "$BASE_URL"

curl_postj action.php "action=settings.write&car-numbering=0%2B1" | check_jsuccess

# Per #288: test cars don't disturb numbering?
curl_postj action.php "action=racer.import&firstname=Test&lastname=Car1&carnumber=901&partition=test&exclude=" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Test&lastname=Car2&carnumber=902&partition=test&exclude=" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Test&lastname=Car3&carnumber=903&partition=test&exclude=" | check_jsuccess

N=1
for NAME in 'Sidney Booth' \
                'Annika Fuentes' \
                'Leia Gordon' \
                'Adonis Olsen' \
                'Porter Mueller' \
                'Raiden Robinson' \
                'Marques Calderon' \
                'Jazmyn Osborn' \
                'Mia Kaufman' \
                'Gunner Richardson' \
                'Brenna Li' \
                'Ronnie Bender' \
                'Marcus Riley' \
                'Jaylen Villarreal' \
                'Camila Morales' \
                'Bryan Pope'; do
        expect_car_number "$NAME" Bears $N
    N=$((N+1))
done

for NAME in 'Jovany Lamb' \
                'Denisse Eaton' \
                'Makenna Berg' \
                'Cali Hendricks' \
                'Heidi Murphy' \
                'Jeramiah Delacruz' \
                'Sarah Esparza' \
                'Gilbert Ingram' \
                'Kailee Navarro' \
                'Yamilet Cruz' \
                'Yazmin Harrington' \
                'Daniel Mcneil' \
                'Edwin Diaz' \
                'Pranav Pena' \
                'Ayden Duke' \
                'Juliette Nunez' \
                'Rylee Yang' \
                'Zavier Mata' \
                'Serena Harding' \
                'Zariah Nolan' \
                'Mauricio Bartlett' \
                'Oscar Jacobson' \
                'Elisha Avery' \
                'Andrew Ferguson' \
                'Macey Mosley' \
                'Francisco Ross' \
                'Grant Hubbard' \
                'Ashlyn Moses' \
                'Jacoby Villanueva' \
                'Jordon Velasquez' \
                'Urijah Hobbs' \
                'Nayeli Medina' \
                'Elliot Mathis' \
                'Tamara Sellers' \
                'Turner Kane' \
                'Yoselin Sharp' \
                'Chaz Neal' \
                'Chanel Valencia' \
                'Zaria Villanueva' \
                'Armando Sheppard' \
                'Ashlee David' \
                'Alyssa Fuller' \
                'Jaylynn Rasmussen' \
                'Rayna Pierce' \
                'Nadia Monroe' \
                'Ashlyn Davis' \
                'Andreas Best' \
                'Antoine Beasley' \
                'Malia Lane' \
                'Leanna Vaughan' \
                'Vaughn Mccann' \
                'Alena Day' \
                'Giana Fisher' \
                'Megan Wu' \
                'Andreas Bentley' \
                'Adriana Hays' \
                'Heidi Phelps' \
                'Juliette Dunlap' \
                'Arnav Burke' \
                'Duncan Petty' \
                'Naima Cortez' \
                'Anabella Curtis' \
                'Zavier Salazar' \
                'Alexus Cain' \
                'Kadence Braun' \
                'Paisley Mcneil' \
                'Skylar Herrera' \
                'Asia Bond' \
                'Ashley Richards' \
                'Maliyah Sawyer' \
                'Ruben Petersen' \
                'Richard Terrell' \
                'Alyvia Moyer' \
                'Brycen Herman' \
                'Eliza French' \
                'Eileen Mccall' \
                'Jaime Bruce' \
                'Anna Blair' \
                'Joaquin Luna' \
                'Gracelyn Dominguez' \
                'Gillian Meza' \
                'Leonel Duran' \
                'Nathanael Cardenas' \
                'Valentina Beck' \
                'Danielle Leblanc' \
                'Alberto Snow' \
                'Arely Estes' \
                'Jolie Singleton' \
                'Donte Hancock' \
                'Yaretzi Flynn' \
                'Damien Goodman' \
                'Priscilla Rush' \
                'Essence Hess' \
                'Kayden Stevens' \
                'Carter Houston' \
                'Dahlia Drake' \
                'Angelo Dean' \
                'Violet Rogers' \
                'Willie Novak' \
                'Kaya Decker' \
                'Isis Sexton' \
                'Alani Petty' \
                'Niko Stephens' \
                'Marlie Decker' ; do
    expect_car_number "$NAME" Webelos $N
    N=$((N+1))
done
