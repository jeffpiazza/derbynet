#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# Assumes:  import-roster.sh

curl_postj action.php "action=settings.write&unused-lane-mask=0&n-lanes=4" | check_jsuccess

# Check in everyone and race all the rounds
curl_postj action.php "action=racer.bulk&who=all&what=checkin&value=1" | check_jsuccess

curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=2" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=3" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=4" | check_jsuccess
curl_postj action.php "action=schedule.generate&roundid=5" | check_jsuccess


curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess
run_heat 1 1   101:3.488 111:3.656 126:3.360 146:3.518
run_heat 1 2   106:3.393 116:3.273 131:3.346 151:3.293
run_heat 1 3   111:3.339 121:3.698 136:3.844 156:3.465
run_heat 1 4   116:3.260 126:3.351 141:3.441 161:3.672
run_heat 1 5   121:3.738 131:3.392 146:3.717 166:3.682
run_heat 1 6   126:3.116 136:3.591 151:3.553 171:3.800
run_heat 1 7   131:3.026 141:3.381 156:3.464 176:3.843
run_heat 1 8   136:3.233 146:3.127 161:3.630 181:3.694
run_heat 1 9   141:3.845 151:3.144 166:3.859 101:3.090
run_heat 1 10  146:3.345 156:3.515 171:3.708 106:3.150
run_heat 1 11  151:3.448 161:3.127 176:3.407 111:3.005
run_heat 1 12  156:3.429 166:3.329 181:3.638 116:3.074
run_heat 1 13  161:3.370 171:3.475 101:3.232 121:3.869
run_heat 1 14  166:3.291 176:3.422 106:3.571 126:3.015
run_heat 1 15  171:3.759 181:3.874 111:3.042 131:3.261
run_heat 1 16  176:3.452 101:3.146 116:3.592 136:3.882
run_heat 1 17  181:3.819 106:3.065 121:3.761 141:3.551 x


# Lions & Tigers standings here:
#
# T1 Edgardo (T)   26
# L  Adolfo (L)    1
# L  Felton (L)    31
# L  Carroll (L)  11
# T2 Ben          6
# T3 Danial       16
# L  Levi (L)    51
# T4 Kelvin    46
# L  Owen (L)    61
# T5 Michal    56
# T6 Toby    76
# L  Raymon (L)    66
# C1 Jesse (C)    41
# C2 Herb (C)    36
# T7 Rodrigo    71
# T8 Willard    81
# C3 Derick (C)    21

curl_postj action.php "action=heat.select&roundid=2&now_racing=1" | check_jsuccess
run_heat 2 1   202:3.711 212:3.108 227:3.420 247:3.463
run_heat 2 2   207:3.397 217:3.019 232:3.792 252:3.081
run_heat 2 3   212:3.710 222:3.874 237:3.474 257:3.757
run_heat 2 4   217:3.140 227:3.717 242:3.816 262:3.197
run_heat 2 5   222:3.104 232:3.395 247:3.402 267:3.868
run_heat 2 6   227:3.788 237:3.473 252:3.666 272:3.803
run_heat 2 7   232:3.325 242:3.485 257:3.645 277:3.643
run_heat 2 8   237:3.878 247:3.302 262:3.033 282:3.393
run_heat 2 9   242:3.751 252:3.123 267:3.489 202:3.344
run_heat 2 10  247:3.178 257:3.648 272:3.843 207:3.805
run_heat 2 11  252:3.647 262:3.093 277:3.248 212:3.845
run_heat 2 12  257:3.085 267:3.576 282:3.474 217:3.451
run_heat 2 13  262:3.563 272:3.702 202:3.107 222:3.841
run_heat 2 14  267:3.488 277:3.266 207:3.650 227:3.687
run_heat 2 15  272:3.245 282:3.477 212:3.846 232:3.571
run_heat 2 16  277:3.291 202:3.811 217:3.786 237:3.189
run_heat 2 17  282:3.642 207:3.182 222:3.571 242:3.311 x

# White's Wolves standings:
# Pat Petrone(62)
# Kelvin Knapp(47)
# Darrell & Darrell(17)
# Tracy(77)
# Lewis(52)
# Angelo(2)
# Willard(82)
# Ian(37)
# Blake(7)
# Freddie(32)
# Mohamed(57)
# Josh(42)
# Dexter(22)
# Renaldo(67)
# Christopher(12)
# Royce(72)
# Elliot Eastman(27)

curl_postj action.php "action=heat.select&roundid=3&now_racing=1" | check_jsuccess
run_heat 3 1   303:3.848 313:3.865 328:3.250 363:3.017
run_heat 3 2   308:3.507 318:3.105 333:3.551 368:3.748
run_heat 3 3   313:3.897 323:3.681 338:3.482 373:3.398
run_heat 3 4   318:3.522 328:3.647 343:3.546 378:3.141
run_heat 3 5   323:3.718 333:3.347 348:3.464 303:3.650
run_heat 3 6   328:3.244 338:3.847 353:3.872 308:3.662
run_heat 3 7   333:3.067 343:3.723 358:3.121 313:3.448
run_heat 3 8   338:3.076 348:3.806 363:3.049 318:3.530
run_heat 3 9   343:3.563 353:3.675 368:3.495 323:3.798
run_heat 3 10  348:3.846 358:3.124 373:3.810 328:3.638
run_heat 3 11  353:3.786 363:3.647 378:3.124 333:3.501
run_heat 3 12  358:3.410 368:3.361 303:3.760 338:3.241
run_heat 3 13  363:3.291 373:3.620 308:3.444 343:3.307
run_heat 3 14  368:3.554 378:3.571 313:3.629 348:3.765
run_heat 3 15  373:3.764 303:3.636 318:3.001 353:3.131
run_heat 3 16  378:3.535 308:3.205 323:3.207 358:3.249 x

# Bears and Freres standings:
#
# Nelson No(58)
# Pete(63)
# Darrin(18)
# Travis(78)
# Freeman(33)
# Jackson(38)
# Ethan(28)
# Bruce(8)
# Juan(43)
# Reuben(68)
# Domingo(23)
# Lyman(53)
# Scottie(73)
# Clark(13)
# Kory(48)
# Antoine Akiyama(3)


curl_postj action.php "action=heat.select&roundid=4&now_racing=1" | check_jsuccess
run_heat 4 1   501:3.756 503:3.737 506:3.639 513:3.892
run_heat 4 2   502:3.141 504:3.306 507:3.742 514:3.087
run_heat 4 3   503:3.197 505:3.532 508:3.436 515:3.064
run_heat 4 4   504:3.364 506:3.104 509:3.488 516:3.682
run_heat 4 5   505:3.105 507:3.307 510:3.168 501:3.099
run_heat 4 6   506:3.243 508:3.427 511:3.360 502:3.207
run_heat 4 7   507:3.591 509:3.581 512:3.517 503:3.733
run_heat 4 8   508:3.433 510:3.321 513:3.165 504:3.056
run_heat 4 9   509:3.163 511:3.613 514:3.133 505:3.880
run_heat 4 10  510:3.714 512:3.176 515:3.484 506:3.696
run_heat 4 11  511:3.078 513:3.281 516:3.362 507:3.177
run_heat 4 12  512:3.696 514:3.129 501:3.218 508:3.335
run_heat 4 13  513:3.680 515:3.102 502:3.641 509:3.382
run_heat 4 14  514:3.035 516:3.297 503:3.107 510:3.762
run_heat 4 15  515:3.324 501:3.861 504:3.045 511:3.767
run_heat 4 16  516:3.499 502:3.468 505:3.388 512:3.281 x

# Webelos (Webes standings:
#
# Rex(69)
# Denny(19)
# Sean(74)
# Byron(9)
# Judson(44)
# Jamison(39)
# Norbert(59)
# Emory(29)
# Cletus(14)
# Gregg(34)
# Markus(54)
# Vincent(79)
# Dorian(24)
# Arden(4)
# Kris(49)
# Philip(64)


curl_postj action.php "action=heat.select&roundid=5&now_racing=1" | check_jsuccess
run_heat 5 1   405:3.315 415:3.620 430:3.242 465:3.455
run_heat 5 2   410:3.773 420:3.022 435:3.638 470:3.080
run_heat 5 3   415:3.698 425:3.597 440:3.512 475:3.853
run_heat 5 4   420:3.050 430:3.831 445:3.416 480:3.148
run_heat 5 5   425:3.214 435:3.723 450:3.201 405:3.595
run_heat 5 6   430:3.527 440:3.377 455:3.843 410:3.735
run_heat 5 7   435:3.120 445:3.092 460:3.881 415:3.789
run_heat 5 8   440:3.601 450:3.335 465:3.705 420:3.058
run_heat 5 9   445:3.161 455:3.394 470:3.769 425:3.190
run_heat 5 10  450:3.205 460:3.642 475:3.816 430:3.629
run_heat 5 11  455:3.563 465:3.403 480:3.396 435:3.029
run_heat 5 12  460:3.329 470:3.067 405:3.418 440:3.468
run_heat 5 13  465:3.610 475:3.679 410:3.837 445:3.388
run_heat 5 14  470:3.867 480:3.704 415:3.841 450:3.610
run_heat 5 15  475:3.267 405:3.404 420:3.357 455:3.224
run_heat 5 16  480:3.646 410:3.184 425:3.678 460:3.263 x


# Arrows <<-<< Standings:
#
# Derek(20)
# Julian(45)
# Lanny(50)
# Harley(35)
# Earnest(25)
# Barney(5)
# Robbie(70)
# Weston(80)
# Jed(40)
# Marlon(55)
# Numbers(60)
# Porter(65)
# Enoch(30)
# Carey(10)
# Timmy(75)
# Cañumil(15)
