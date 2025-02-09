<?php

require_once('inc/schema_version.inc');

// car-numbering: m+s
//
// m: per-segment multiplier is 100 (default) or 0.
// v: smallest value is 101 (default) or 1
function read_car_numbering_values() {
  return explode('+', read_raceinfo('car-numbering', '100+101'));
}

// Returns an assoc of partitionid => next carnumber for that partition
function next_car_numbers() {
  global $db;
  if (schema_version() >= PARTITION_SCHEMA) {
    list($mult, $smallest) = read_car_numbering_values();    
    $mult = intval($mult);
    $smallest = intval($smallest);

    $stmt = $db->prepare(
      'SELECT partitionid,'
      .'      COALESCE(1 + (SELECT MIN(carnumber) FROM RegistrationInfo AS reg'
      .'                    WHERE carnumber >= (:sm1 + (sortorder - 1) * :m1) - 1'
      .'                      AND (:m2 = 0 OR carnumber <  (:sm2 + sortorder * :m3))'
      .'                      AND NOT EXISTS(SELECT 1 FROM RegistrationInfo AS conflict'
      .'                                     WHERE conflict.carnumber = 1 + reg.carnumber)),'
      .'               :sm3 + (sortorder - 1) * :m4)'
      .'       AS next_carnumber'
      .' FROM Partitions'
      .' ORDER BY sortorder');
    $stmt->execute(array(':sm1' => $smallest,
                         ':sm2' => $smallest,
                         ':sm3' => $smallest,
                         ':m1' => $mult,
                         ':m2' => $mult,
                         ':m3' => $mult,
                         ':m4' => $mult));
    return $stmt->fetchAll(PDO::FETCH_ASSOC);
  } else {
    return $db->query(
      'SELECT classid AS partitionid,'
      .' 1 + MAX(carnumber) AS next_carnumber'
      .' FROM RegistrationInfo'
      .' GROUP BY classid')->fetchAll(PDO::FETCH_ASSOC);
  }
}

function next_car_number_for_partition($partitionid) {
  global $db;
  list($mult, $smallest) = read_car_numbering_values();

  return read_single_value('SELECT'
                           .' (CASE NOT EXISTS(SELECT 1 FROM RegistrationInfo AS conflict'
                           .'                  WHERE conflict.carnumber = lbound)'
                           .'  WHEN 1 THEN lbound'
                           .'  ELSE 1 + MIN(carnumber) END'
                           .' ) AS next_carnumber'
                           .' FROM (SELECT partitionid,'
                           .'              CAST(:mult0 AS INTEGER) AS mult,'
                           .'              :smallest  + (sortorder - 1) * :mult  AS lbound,'
                           .'              :smallest2 + (sortorder    ) * :mult2 AS ubound'
                           .'              FROM Partitions'
                           .'              WHERE partitionid = :partitionid'
                           .') AS part'
                           .' LEFT JOIN (SELECT carnumber FROM RegistrationInfo) AS reg'
                           .'  ON  carnumber >= lbound'
                           .'  AND (mult = 0 OR carnumber <  ubound)'
                           .'  AND NOT EXISTS(SELECT 1 FROM RegistrationInfo conflict'
                           .'                  WHERE conflict.carnumber = 1 + reg.carnumber)',
                           array(':partitionid' => intval($partitionid),
                                 ':mult0' => intval($mult),
                                 ':mult' => intval($mult),
                                 ':smallest' => intval($smallest),
                                 ':mult2' => intval($mult),
                                 ':smallest2' => intval($smallest)));
}

// Returns an integer (the first car number) if we're distinguishing by
// partition, otherwise an assoc mapping partitionid to first car number to
// assign in that partition.
function base_car_number_by_partition() {
  global $db;
  list($mult, $smallest) = read_car_numbering_values();
  if ($mult == 0) {
    return intval($smallest);
  } else {
    $stmt = $db->prepare('SELECT partitionid, :smallest + (sortorder - 1) * :mult'
                         .' FROM Partitions');
    $stmt->execute(array(':mult' => $mult,
                         ':smallest' => $smallest));
    return $stmt->fetchAll(PDO::FETCH_KEY_PAIR);
  }
}
  

?>
