<awards>
<?php

require_once('inc/awards.inc');

require('ajax/query.class.list.inc');

foreach ($db->query('SELECT * FROM AwardTypes ORDER BY awardtypeid') as $atype) {
  echo '<awardtype';
  echo ' awardtypeid="'.$atype['awardtypeid'].'"';
  echo ' awardtype="'.htmlspecialchars($atype['awardtype'], ENT_QUOTES, 'UTF-8').'"';
  echo "/>\n";
}

foreach (all_awards() as $award) {
  echo '<award';
  echo ' awardid="'.$award['awardid'].'"';
  echo ' awardname="'.htmlspecialchars($award['awardname'], ENT_QUOTES, 'UTF-8').'"';
  echo ' awardtypeid="'.$award['awardtypeid'].'"';
  echo ' classid="'.@$award['classid'].'"';
  echo ' rankid="'.@$award['rankid'].'"';
  echo ' racerid="'.$award['racerid'].'"';
  echo ' firstname="'.htmlspecialchars($award['firstname'], ENT_QUOTES, 'UTF-8').'"';
  echo ' lastname="'.htmlspecialchars($award['lastname'], ENT_QUOTES, 'UTF-8').'"';
  echo ' carnumber="'.$award['carnumber'].'"';
  echo ' sort="'.$award['sort'].'"';
  echo "/>\n";
}
?>
</awards>