#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"

# Check in everyone and race all the rounds
curl_post action.php "action=racer.bulk&who=all&what=checkin&value=1" | check_success

curl_post action.php "action=schedule.generate&roundid=1" | check_success
curl_post action.php "action=schedule.generate&roundid=2" | check_success
curl_post action.php "action=schedule.generate&roundid=3" | check_success
curl_post action.php "action=schedule.generate&roundid=4" | check_success
curl_post action.php "action=schedule.generate&roundid=5" | check_success

echo Starting run_heat

curl_post action.php "action=select-heat&roundid=1&now_racing=1" | check_success
run_heat	1	1	3.488	3.656	3.36	3.518
run_heat	1	2	3.393	3.273	3.346	3.293
run_heat	1	3	3.339	3.698	3.844	3.465
run_heat	1	4	3.26	3.351	3.441	3.672
run_heat	1	5	3.738	3.392	3.717	3.682
run_heat	1	6	3.116	3.591	3.553	3.8
run_heat	1	7	3.026	3.381	3.464	3.843
run_heat	1	8	3.233	3.127	3.63	3.694
run_heat	1	9	3.845	3.144	3.859	3.09
run_heat	1	10	3.345	3.515	3.708	3.15
run_heat	1	11	3.448	3.127	3.407	3.005
run_heat	1	12	3.429	3.329	3.638	3.074
run_heat	1	13	3.37	3.475	3.232	3.869
run_heat	1	14	3.291	3.422	3.571	3.015
run_heat	1	15	3.759	3.874	3.042	3.261
run_heat	1	16	3.452	3.146	3.592	3.882
run_heat	1	17	3.819	3.065	3.761	3.551  x

curl_post action.php "action=select-heat&roundid=2&now_racing=1" | check_success
run_heat	2	1	3.711	3.108	3.42	3.463
run_heat	2	2	3.397	3.019	3.792	3.081
run_heat	2	3	3.71	3.874	3.474	3.757
run_heat	2	4	3.14	3.717	3.816	3.197
run_heat	2	5	3.104	3.395	3.402	3.868
run_heat	2	6	3.788	3.473	3.666	3.803
run_heat	2	7	3.325	3.485	3.645	3.643
run_heat	2	8	3.878	3.302	3.033	3.393
run_heat	2	9	3.751	3.123	3.489	3.344
run_heat	2	10	3.178	3.648	3.843	3.805
run_heat	2	11	3.647	3.093	3.248	3.845
run_heat	2	12	3.085	3.576	3.474	3.451
run_heat	2	13	3.563	3.702	3.107	3.841
run_heat	2	14	3.488	3.266	3.65	3.687
run_heat	2	15	3.245	3.477	3.846	3.571
run_heat	2	16	3.291	3.811	3.786	3.189
run_heat	2	17	3.642	3.182	3.571	3.311 x

curl_post action.php "action=select-heat&roundid=3&now_racing=1" | check_success
run_heat	3	1	3.848	3.865	3.25	3.017
run_heat	3	2	3.507	3.105	3.551	3.748
run_heat	3	3	3.897	3.681	3.482	3.398
run_heat	3	4	3.522	3.647	3.546	3.141
run_heat	3	5	3.718	3.347	3.464	3.65
run_heat	3	6	3.244	3.847	3.872	3.662
run_heat	3	7	3.067	3.723	3.121	3.448
run_heat	3	8	3.076	3.806	3.049	3.53
run_heat	3	9	3.563	3.675	3.495	3.798
run_heat	3	10	3.846	3.124	3.81	3.638
run_heat	3	11	3.786	3.647	3.124	3.501
run_heat	3	12	3.41	3.361	3.76	3.241
run_heat	3	13	3.291	3.62	3.444	3.307
run_heat	3	14	3.554	3.571	3.629	3.765
run_heat	3	15	3.764	3.636	3.001	3.131
run_heat	3	16	3.535	3.205	3.207	3.249 x

curl_post action.php "action=select-heat&roundid=4&now_racing=1" | check_success
run_heat	4	1	3.756	3.737	3.639	3.892
run_heat	4	2	3.141	3.306	3.742	3.087
run_heat	4	3	3.197	3.532	3.436	3.064
run_heat	4	4	3.364	3.104	3.488	3.682
run_heat	4	5	3.105	3.307	3.168	3.099
run_heat	4	6	3.243	3.427	3.36	3.207
run_heat	4	7	3.591	3.581	3.517	3.733
run_heat	4	8	3.433	3.321	3.165	3.056
run_heat	4	9	3.163	3.613	3.133	3.88
run_heat	4	10	3.714	3.176	3.484	3.696
run_heat	4	11	3.078	3.281	3.362	3.177
run_heat	4	12	3.696	3.129	3.218	3.335
run_heat	4	13	3.68	3.102	3.641	3.382
run_heat	4	14	3.035	3.297	3.107	3.762
run_heat	4	15	3.324	3.861	3.045	3.767
run_heat	4	16	3.499	3.468	3.388	3.281 x

curl_post action.php "action=select-heat&roundid=5&now_racing=1" | check_success
run_heat	5	1	3.315	3.62	3.242	3.455
run_heat	5	2	3.773	3.022	3.638	3.08
run_heat	5	3	3.698	3.597	3.512	3.853
run_heat	5	4	3.05	3.831	3.416	3.148
run_heat	5	5	3.214	3.723	3.201	3.595
run_heat	5	6	3.527	3.377	3.843	3.735
run_heat	5	7	3.12	3.092	3.881	3.789
run_heat	5	8	3.601	3.335	3.705	3.058
run_heat	5	9	3.161	3.394	3.769	3.19
run_heat	5	10	3.205	3.642	3.816	3.629
run_heat	5	11	3.563	3.403	3.396	3.029
run_heat	5	12	3.329	3.067	3.418	3.468
run_heat	5	13	3.61	3.679	3.837	3.388
run_heat	5	14	3.867	3.704	3.841	3.61
run_heat	5	15	3.267	3.404	3.357	3.224
run_heat	5	16	3.646	3.184	3.678	3.263 x

## Create "Younger Finals" aggregate of roundid 1,2 and race the round
curl_post action.php "action=roster.new&top=4&bucketed=1&roundid_1=1&roundid_2=1&classname=Younger%20Finals" | check_success
curl_post action.php "action=schedule.generate&roundid=6" | check_success
curl_post action.php "action=select-heat&roundid=6&now_racing=1" | check_success
run_heat	6	1	3.103	3.762	3.359	3.471
run_heat	6	2	3.757	3.635	3.085	3.328
run_heat	6	3	3.586	3.749	3.095	3.494
run_heat	6	4	3.095	3.645	3.836	3.54
run_heat	6	5	3.087	3.278	3.702	3.135
run_heat	6	6	3.602	3.405	3.632	3.523
run_heat	6	7	3.685	3.136	3.737	3.419
run_heat	6	8	3.000	3.477	3.67	3.512 x

## Create "Older Finals" aggregate of roundid 3,4,5, and race
curl_post action.php "action=roster.new&top=4&bucketed=1&roundid_3=1&roundid_4=1&roundid_5=1&classname=Older%20Finals" | check_success
curl_post action.php "action=schedule.generate&roundid=7" | check_success
curl_post action.php "action=select-heat&roundid=7&now_racing=1" | check_success
run_heat	7	1	3.85	3.145	3.849	3.288
run_heat	7	2	3.706	3.212	3.343	3.711
run_heat	7	3	3.282	3.51	3.703	3.653
run_heat	7	4	3.785	3.835	3.264	3.024
run_heat	7	5	3.081	3.502	3.65	3.256
run_heat	7	6	3.12	3.286	3.209	3.89
run_heat	7	7	3.352	3.673	3.355	3.039
run_heat	7	8	3.11	3.026	3.401	3.152
run_heat	7	9	3.756	3.763	3.38	3.056
run_heat	7	10	3.45	3.692	3.079	3.329
run_heat	7	11	3.003	3.79	3.47	3.199
run_heat	7	12	3.563	3.857	3.255	3.742 x

## Race a second round of Older Finals
curl_post action.php "action=roster.new&top=8&roundid=7" | check_success
curl_post action.php "action=schedule.generate&roundid=8" | check_success
curl_post action.php "action=select-heat&roundid=8&now_racing=1" | check_success

run_heat	8	1	3.862	3.887	3.162	3.234
run_heat	8	2	3.329	3.401	3.554	3.015
run_heat	8	3	3.674	3.837	3.102	3.5
run_heat	8	4	3.792	3.685	3.096	3.53
run_heat	8	5	3.819	3.501	3.624	3.451
run_heat	8	6	3.183	3.007	3.227	3.665
run_heat	8	7	3.37	3.512	3.448	3.486
run_heat	8	8	3.028	3.104	3.049	3.301 x

## Create a final final of the other two GF's
curl_post action.php "action=roster.new&top=4&bucketed=1&roundid_8=1&roundid_6=1&classname=Final%20Finals" | check_success
curl_post action.php "action=schedule.generate&roundid=9" | check_success
curl_post action.php "action=select-heat&roundid=9&now_racing=1" | check_success

run_heat	9	1	3.061	3.857	3.565	3.674
run_heat	9	2	3.825	3.672	3.592	3.106
run_heat	9	3	3.392	3.409	3.651	3.577
run_heat	9	4	3.65	3.132	3.415	3.85
run_heat	9	5	3.895	3.052	3.636	3.096
run_heat	9	6	3.598	3.415	3.625	3.831
run_heat	9	7	3.059	3.54	3.306	3.714
run_heat	9	8	3.563	3.657	3.648	3.895 x

