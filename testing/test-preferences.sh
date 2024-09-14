#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator


RESET_SOURCE=preferences `dirname $0`/reset-database.sh "$BASE_URL"

curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
     -X POST \
     -F action=preferences.upload \
     -F prefs="@-;filename=example.pref" \
     -F MAX_FILE_SIZE=2000000 <<EOF \
    | tee $DEBUG_CURL | check_jsuccess
# Preferences written 2024-04-27

## Groups
GROUP = Lions & Tigers
  SEGMENT = Lion
  SEGMENT = Lion Pro-Stock
  SEGMENT = Tiger
  SEGMENT = Tiger Pro-Stock
ENDGROUP
GROUP = Webelos I and II
  SEGMENT = Webelos
  SEGMENT = Webelos Pro-Stock
  SEGMENT = Arrow of Light
  SEGMENT = Arrow of Light Pro-Stock
ENDGROUP
GROUP = Wolves & Bears
  SEGMENT = Bear
  SEGMENT = Bear Pro-Stock
  SEGMENT = Wolf
  SEGMENT = Wolf Pro-Stock
ENDGROUP

## Awards

## Settings
n-lanes = 4
group-formation-rule = custom
use-subgroups
partition-label = Rank
supergroup-label = Council
n-pack-trophies = 0
n-den-trophies = 0
n-rank-trophies = 3
racing_scene = 4
warn-no-timer

## Aggregates
AGGREGATE = All Lions
  CONSTITUENTSUBGROUP = Lion
  CONSTITUENTSUBGROUP = Lion Pro-Stock
ENDAGGREGATE
AGGREGATE = All Tigers
  CONSTITUENTSUBGROUP = Tiger
  CONSTITUENTSUBGROUP = Tiger Pro-Stock
ENDAGGREGATE
AGGREGATE = All Wolves
  CONSTITUENTSUBGROUP = Wolf
  CONSTITUENTSUBGROUP = Wolf Pro-Stock
ENDAGGREGATE
AGGREGATE = All Bears
  CONSTITUENTSUBGROUP = Bear
  CONSTITUENTSUBGROUP = Bear Pro-Stock
ENDAGGREGATE
AGGREGATE = All Webelos
  CONSTITUENTSUBGROUP = Webelos
  CONSTITUENTSUBGROUP = Webelos Pro-Stock
ENDAGGREGATE
AGGREGATE = All Arrow-of-Light
  CONSTITUENTSUBGROUP = Arrow of Light
  CONSTITUENTSUBGROUP = Arrow of Light Pro-Stock
ENDAGGREGATE

EOF


# TODO
curl_getj "action.php?query=poll.coordinator" > poll-coordinator.json

# curl_getj "action.php?query=poll&values=awards,classes,partitions,race-structure,rounds"

diff <(curl_getj "action.php?query=poll&values=classes,partitions,rounds") - <<EOF
{
    "classes": [
        {
            "classid": 1,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "Lions & Tigers",
            "subgroups": [
                {
                    "rankid": 1,
                    "count": 0,
                    "name": "Lion"
                },
                {
                    "rankid": 2,
                    "count": 0,
                    "name": "Lion Pro-Stock"
                },
                {
                    "rankid": 3,
                    "count": 0,
                    "name": "Tiger"
                },
                {
                    "rankid": 4,
                    "count": 0,
                    "name": "Tiger Pro-Stock"
                }
            ]
        },
        {
            "classid": 2,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "Webelos I and II",
            "subgroups": [
                {
                    "rankid": 5,
                    "count": 0,
                    "name": "Webelos"
                },
                {
                    "rankid": 6,
                    "count": 0,
                    "name": "Webelos Pro-Stock"
                },
                {
                    "rankid": 7,
                    "count": 0,
                    "name": "Arrow of Light"
                },
                {
                    "rankid": 8,
                    "count": 0,
                    "name": "Arrow of Light Pro-Stock"
                }
            ]
        },
        {
            "classid": 3,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "Wolves & Bears",
            "subgroups": [
                {
                    "rankid": 9,
                    "count": 0,
                    "name": "Bear"
                },
                {
                    "rankid": 10,
                    "count": 0,
                    "name": "Bear Pro-Stock"
                },
                {
                    "rankid": 11,
                    "count": 0,
                    "name": "Wolf"
                },
                {
                    "rankid": 12,
                    "count": 0,
                    "name": "Wolf Pro-Stock"
                }
            ]
        },
        {
            "classid": 4,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "All Lions",
            "aggregate-by-subgroup": true,
            "subgroups": [],
            "constituents": [
                {
                    "classid": 1,
                    "name": "Lions & Tigers"
                }
            ],
            "constituent-ranks": [
                {
                    "rankid": 1,
                    "name": "Lion",
                    "classid": 1,
                    "class": "Lions & Tigers"
                },
                {
                    "rankid": 2,
                    "name": "Lion Pro-Stock",
                    "classid": 1,
                    "class": "Lions & Tigers"
                }
            ]
        },
        {
            "classid": 5,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "All Tigers",
            "aggregate-by-subgroup": true,
            "subgroups": [],
            "constituents": [
                {
                    "classid": 1,
                    "name": "Lions & Tigers"
                }
            ],
            "constituent-ranks": [
                {
                    "rankid": 3,
                    "name": "Tiger",
                    "classid": 1,
                    "class": "Lions & Tigers"
                },
                {
                    "rankid": 4,
                    "name": "Tiger Pro-Stock",
                    "classid": 1,
                    "class": "Lions & Tigers"
                }
            ]
        },
        {
            "classid": 6,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "All Wolves",
            "aggregate-by-subgroup": true,
            "subgroups": [],
            "constituents": [
                {
                    "classid": 3,
                    "name": "Wolves & Bears"
                }
            ],
            "constituent-ranks": [
                {
                    "rankid": 11,
                    "name": "Wolf",
                    "classid": 3,
                    "class": "Wolves & Bears"
                },
                {
                    "rankid": 12,
                    "name": "Wolf Pro-Stock",
                    "classid": 3,
                    "class": "Wolves & Bears"
                }
            ]
        },
        {
            "classid": 7,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "All Bears",
            "aggregate-by-subgroup": true,
            "subgroups": [],
            "constituents": [
                {
                    "classid": 3,
                    "name": "Wolves & Bears"
                }
            ],
            "constituent-ranks": [
                {
                    "rankid": 9,
                    "name": "Bear",
                    "classid": 3,
                    "class": "Wolves & Bears"
                },
                {
                    "rankid": 10,
                    "name": "Bear Pro-Stock",
                    "classid": 3,
                    "class": "Wolves & Bears"
                }
            ]
        },
        {
            "classid": 8,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "All Webelos",
            "aggregate-by-subgroup": true,
            "subgroups": [],
            "constituents": [
                {
                    "classid": 2,
                    "name": "Webelos I and II"
                }
            ],
            "constituent-ranks": [
                {
                    "rankid": 5,
                    "name": "Webelos",
                    "classid": 2,
                    "class": "Webelos I and II"
                },
                {
                    "rankid": 6,
                    "name": "Webelos Pro-Stock",
                    "classid": 2,
                    "class": "Webelos I and II"
                }
            ]
        },
        {
            "classid": 9,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "All Arrow-of-Light",
            "aggregate-by-subgroup": true,
            "subgroups": [],
            "constituents": [
                {
                    "classid": 2,
                    "name": "Webelos I and II"
                }
            ],
            "constituent-ranks": [
                {
                    "rankid": 7,
                    "name": "Arrow of Light",
                    "classid": 2,
                    "class": "Webelos I and II"
                },
                {
                    "rankid": 8,
                    "name": "Arrow of Light Pro-Stock",
                    "classid": 2,
                    "class": "Webelos I and II"
                }
            ]
        }
    ],
    "partitions": [
        {
            "partitionid": 1,
            "name": "Lion",
            "sortorder": 1,
            "count": 0,
            "classids": [
                1
            ],
            "rankids": [
                1
            ]
        },
        {
            "partitionid": 2,
            "name": "Lion Pro-Stock",
            "sortorder": 2,
            "count": 0,
            "classids": [
                1
            ],
            "rankids": [
                2
            ]
        },
        {
            "partitionid": 3,
            "name": "Tiger",
            "sortorder": 3,
            "count": 0,
            "classids": [
                1
            ],
            "rankids": [
                3
            ]
        },
        {
            "partitionid": 4,
            "name": "Tiger Pro-Stock",
            "sortorder": 4,
            "count": 0,
            "classids": [
                1
            ],
            "rankids": [
                4
            ]
        },
        {
            "partitionid": 5,
            "name": "Webelos",
            "sortorder": 5,
            "count": 0,
            "classids": [
                2
            ],
            "rankids": [
                5
            ]
        },
        {
            "partitionid": 6,
            "name": "Webelos Pro-Stock",
            "sortorder": 6,
            "count": 0,
            "classids": [
                2
            ],
            "rankids": [
                6
            ]
        },
        {
            "partitionid": 7,
            "name": "Arrow of Light",
            "sortorder": 7,
            "count": 0,
            "classids": [
                2
            ],
            "rankids": [
                7
            ]
        },
        {
            "partitionid": 8,
            "name": "Arrow of Light Pro-Stock",
            "sortorder": 8,
            "count": 0,
            "classids": [
                2
            ],
            "rankids": [
                8
            ]
        },
        {
            "partitionid": 9,
            "name": "Bear",
            "sortorder": 9,
            "count": 0,
            "classids": [
                3
            ],
            "rankids": [
                9
            ]
        },
        {
            "partitionid": 10,
            "name": "Bear Pro-Stock",
            "sortorder": 10,
            "count": 0,
            "classids": [
                3
            ],
            "rankids": [
                10
            ]
        },
        {
            "partitionid": 11,
            "name": "Wolf",
            "sortorder": 11,
            "count": 0,
            "classids": [
                3
            ],
            "rankids": [
                11
            ]
        },
        {
            "partitionid": 12,
            "name": "Wolf Pro-Stock",
            "sortorder": 12,
            "count": 0,
            "classids": [
                3
            ],
            "rankids": [
                12
            ]
        }
    ],
    "rounds": [
        {
            "roundid": 1,
            "classid": 1,
            "class": "Lions & Tigers",
            "round": 1,
            "aggregate": false,
            "roster_size": 0,
            "passed": 0,
            "unscheduled": 0,
            "adjustments": [],
            "heats_scheduled": 0,
            "heats_run": 0,
            "name": "Lions & Tigers, Round 1"
        },
        {
            "roundid": 2,
            "classid": 2,
            "class": "Webelos I and II",
            "round": 1,
            "aggregate": false,
            "roster_size": 0,
            "passed": 0,
            "unscheduled": 0,
            "adjustments": [],
            "heats_scheduled": 0,
            "heats_run": 0,
            "name": "Webelos I and II, Round 1"
        },
        {
            "roundid": 3,
            "classid": 3,
            "class": "Wolves & Bears",
            "round": 1,
            "aggregate": false,
            "roster_size": 0,
            "passed": 0,
            "unscheduled": 0,
            "adjustments": [],
            "heats_scheduled": 0,
            "heats_run": 0,
            "name": "Wolves & Bears, Round 1"
        }
    ]
}
EOF
