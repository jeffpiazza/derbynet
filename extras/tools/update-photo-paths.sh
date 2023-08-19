#! /bin/sh

update_expr() {
    FIELD_NAME=$1
    FROM_DIR=$2
    TO_DIR=$3
    echo "SET $FIELD_NAME = SUBSTR($FIELD_NAME, 1, INSTR($FIELD_NAME, '/$FROM_DIR/')) \
|| '/$TO_DIR/' \
|| SUBSTR($FIELD_NAME, INSTR($FIELD_NAME, '/$FROM_DIR/') + LENGTH('/$FROM_DIR/'))"
}


echo

echo UPDATE RegistrationInfo
update_expr imagefile Mayflower MayflowerX
echo WHERE imagefile LIKE \'%/Mayflower/%\'\;
echo

echo UPDATE RegistrationInfo
update_expr carphoto Mayflower MayflowerX
echo WHERE carphoto LIKE \'%/Mayflower/%\'\;
echo

echo UPDATE RaceInfo
update_expr itemvalue Mayflower MayflowerX
echo WHERE itemvalue LIKE \'%/Mayflower/%\' AND itemkey LIKE \'%directory\'\;
