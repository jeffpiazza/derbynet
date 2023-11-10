<?php
require_once('inc/data.inc');
require_once('inc/classes.inc');
require_once('inc/partitions.inc');
require_once('inc/schema_version.inc');
require_once('inc/name-mangler.inc');
require_once('inc/ordinals.inc');
require_once('inc/aggregate_round.inc');

if (read_raceinfo('drop-slowest') && read_raceinfo('scoring', -1) == -1) {
  write_raceinfo('scoring', 1);
}

/////////////////////////////////////////////////////////////////////////////////////
// Categories of standings:
//
// ROUND standings: For a given roundid, it's straightforward to calculate
// standings within the round, provided all the heats have been run.  If not all
// heats have been run, compare each racer's scoring-expression if it's
// well-defined, otherwise treat as 9.999.
//
// (For organic racing classes, the "class standings" presented to the user are
// simply the ROUND standings of the highest-numbered round for the class.  For
// aggregate classes, see below.)
//
// ROUND SUBGROUP standings: Within a given ROUND standings, consider just the
// racers for a particular subgroup.  The fastest member of the subgroup in the
// round is considered first in the ROUND SUBGROUP standings, even if they were
// fifth (say) in the ROUND.
//
// QUALIFIER standings: Define qualifier standings for any aggregate class to be
// the (ordered) union of the standings from the highest round of racing for
// each constituent class.  (If a constituent class is also a nonracing
// aggregate class, then its constituent classes are examined in turn.)  The
// name "qualifier" is to convey the idea that the constituent rounds were
// "qualifiers" for some kind of grand final round.  That is, the QUALIFIER
// standings were effectively used to determine participants in the aggregate
// class' roster.
//
// QUALIFIER standings for an aggregate class that includes a list of rankids
// include only the standings for members of the specified ranks.
//
// Finally, the FULL FIELD standings are defined as follows:
//
//  - In the absence of any aggregate rounds, then take the highest-numbered
//    round from each (native) class, and combine into an aggregate standing.
//    Some classes may have more rounds than others.
//
//  - If there are any aggregate classes, then there are no FULL FIELD standings.
//
// Given those category definitions, we make different standings presentations
// to the user:
//
// CLASS standings presentation: The presentation of standings for a class with
// racing rounds is simply the ROUND standings of the highest-numbered round for
// that class.  This applies both to organic classes, and to aggregate classes
// that have associated racing rounds.  If there are (say) 2 rounds of racing
// for the class, this means that racers eliminated in the first round don't
// have a class standing for the class.
//
// For nonracing aggregate classes, the CLASS standings presentation is simply
// the QUALIFIER standings for that class.
//
// (Note that, for aggregate classes that DO have racing rounds, the QUALIFIER
// standings for the class is identified as qualifier standings, to distinguish
// them from the actual CLASS standings presentation.)
//
// (For aggregate classes with racing rounds, there's a little worry that, after
// populating the roster for the first round for the aggregate class, it's
// possible for one of the constituent classes to construct another round.  In
// that case, the aggregate class qualifying standings wouldn't actually reflect
// the results that were used to select the roster for the aggregate class'
// first racing round.  This situation doesn't really make any sense, and should
// probably be disallowed.)
//
// SUBGROUP standings presentation: Standings for a rank are imposed by the
// standings for the class, which is to say, by the standings for the final
// round for the class, with numbering considering only members of a given rank.
//
/////////////////////////////////////////////////////////////////////////////////////
//
// Each computed standings result is marked by a unique key string; the format
// of the key depends on the category.
function stkey_round($roundid) { return 'st-r'.$roundid; }
// Organic classes
// function stkey_class($classid) { return 'st-c'.$classid; }
// Aggregate classes, racing or nonracing
function stkey_qual($classid) { return 'st-q'.$classid; }
function is_stkey_qual($key) { return substr($key, 0, 4) == 'st-q'; }
// Rank within a class
// function stkey_subgroup($rankid) { return 'st-s'.$rankid; }
// Rank within a particular round
function stkey_round_subgroup($roundid, $rankid) { return 'st-rr'.$roundid.'-'.$rankid; }
function stkey_full_field() { return 'st-ff'; }


/////////////////////////////////////////////////////////////////////////////////////
// The finish places for standings result can be distilled into a "ladder",
// represented as an array of rungs, each rung being an array of racerids; the
// zero'th rung represents first place.  In the absence of ties, each rung holds
// exactly one racerid.  If there's a tie, a rung will contain more than one
// racerid, and one or more subsequent rungs will be empty (e.g., a three-way
// tie rung is followed by two empty rungs).
/////////////////////////////////////////////////////////////////////////////////////

// Assumes we're in the context of a GROUP BY for one racer and one roundid.
// $finishtime and $finishplace are, respectively, the sql expressions for
// accessing the finish time and finish place for the one race result.
//
// For place-based races, instead of scoring by (lowest) finish time, we score
// by finish place, exactly as if it were a time.  E.g., treat 1st place like it
// was 1.00 seconds, 2nd place as 2.00 seconds, and so on.
function sql_scoring_expression($finishtime, $finishplace) {
  global $dbtype;
  $finish = read_raceinfo_boolean('use-points') ? $finishplace.'*1.0' : $finishtime;
  $scoring = read_raceinfo('scoring', 0);
  if ($scoring == 0) {
    return "SUM($finish)/COUNT(*)";
  } else if ($scoring == 1) {
    if ($dbtype == 'sqlite') {
      return"CASE COUNT(*)"
             ." WHEN 1 THEN MAX($finish)"
             ." ELSE (SUM($finish) - MAX($finish))/(COUNT(*) - 1)"
             ." END";
    } else {
      // TODO MS Access SELECT CASE <expr> CASE <cond> <result> CASE ELSE <result> END SELECT
      return "(SUM($finish) - MAX($finish))/(COUNT(*) - 1)";
    }
  } else if ($scoring == 2) {
    return  "MIN($finish)";
  } else {
    return "UNKNOWN_SCORING_METHOD";
  }
}



class StandingsOracle {
  function __construct() {
    $this->use_points = read_raceinfo_boolean('use-points');
    $this->use_groups = use_groups();
    $this->use_subgroups = use_subgroups();
    // 0 for calculate normally (pack-ok)
    // -1 for do not calculate (pack-no)
    // +n for aggregate class n.
    // We only care about whether it's normal calc or not.
    $this->full_field_calc = read_raceinfo('full-field-calc', 0);

    list($classes, $this->classseq,  $this->ranks, $this->rankseq) = classes_and_ranks();
    $this->class_details = $this->make_class_details();
    $this->add_rolls_up_to_relation();
    $this->add_rounds_by_class();

    $this->unfinished_rounds = unfinished_rounds();

    $this->result_summary = $this->make_result_summary();
    // Writes $this->all_standings
    $this->compute_all_standings();
  }

  private $use_points;
  private $use_groups;
  private $use_subgroups;
  private $full_field_calc;

  private $classseq;
  private $class_details;
  private $ranks;
  private $rankseq;

  private $unfinished_rounds;
  private $sql;

  public $result_summary;
  public $all_standings;
  // Maps catalog key to number of entries included in those standings
  public $catalog_counts;

  // Returns classid => {classid:, class:,
  //                     constituents:, rankids:, rolls_up_to:, rounds: ... }
  private function make_class_details() {
    global $db;
    $stmt = $db->prepare('SELECT classid, class,'
                         .(schema_version() >= 3 ? ' constituents, ' : '\'\' AS constituents, ')
                         .(schema_version() >= 5 ? ' rankids ' : '\'\' AS rankids ')
                         .' FROM Classes');
    $stmt->execute();
    $stmt->setFetchMode(PDO::FETCH_ASSOC);

    $result = array();
    foreach ($stmt as $row) {
      $row['constituents'] = array_filter(explode(',', $row['constituents']), 'strlen');
      $row['rankids'] = array_filter(explode(',', $row['rankids']), 'strlen');
      $row['rolls_up_to'] = array();  // Filled in by add_rolls_up_to_relation
      $row['rounds'] = array();  // Filled in by add_rounds_by_class, below
      $result[$row['classid']] = $row;
    }
    return $result;
  }

  private function add_rolls_up_to_relation() {
    // Seed the 'rolls_up_to' relation
    foreach ($this->class_details as $classid => &$cl_details) {
      foreach ($cl_details['constituents'] as $c) {
        $this->class_details[$c]['rolls_up_to'][] = $classid;
      }
    }

    // Propagate 'rolls_up_to' relation across non-racing aggregate classes
    do {
      $repeat = false;
      foreach ($this->class_details as $classid => &$cl_details) {
        foreach ($cl_details['rolls_up_to'] as $c) {
          if (count($this->class_details[$c]['rounds']) == 0) {
            foreach ($this->class_details[$c]['rolls_up_to'] as $q) {
              if (!in_array($q, $cl_details['rolls_up_to'])) {
                $cl_details['rolls_up_to'][] = $q;
                $repeat = true;
              }
            }
          }
        }
      }
    } while ($repeat);
  }

  function add_rounds_by_class() {
    global $db;

    $stmt = $db->query('SELECT roundid, classid, round'
                       .' FROM Rounds'
                       .' ORDER BY round DESC');
    foreach ($stmt as $row) {
      $classid = $row['classid'];
      $this->class_details[$classid]['rounds'][] = array('roundid' => $row['roundid'],
                                                         'round' => $row['round']);
    }
  }

  // Returns a summary for each racer's result within each round they
  // participated in.  The results are ordered best-to-worst across all the
  // racers and rounds in the database.  Each summary element gives:
  //    roundid -- round information
  //    round_classid -- the classid associated with the round -- may be an
  //        aggregate class.
  //    racerid, firstname, lastname, carnumber, carname, classid, rankid -- racer
  //        information
  //    classid, class -- name of racer's (native) class (i.e., not "Grand Finals")
  //    rankid, rank -- name of racer's rank
  //    base -- number of heats run in round
  //    score -- result of the scoring expression
  //    avg -- "average" score in this round, possibly after discarding worst heat
  //    best, worst -- fastest and slowest score in this round
  private function make_result_summary() {
    global $db;

    $scoring_expr = sql_scoring_expression('finishtime', 'finishplace');
    $raw_expr = $this->use_points ? 'finishplace' : 'finishtime';
    if ($this->use_points) {
      // finishplace: 1=1st
      // Points for a given heat is (1 + get_lane_count()) - finishplace, e.g.,
      // 1st place on a 4-lane track is 4 points.
      // Points for N heats is (1 + get_lane_count()) * N - SUM(finishplace)
      $scoring = read_raceinfo('scoring', 0);
      if ($scoring == 0) {
        $avg_expr = (1 + get_lane_count()).' * COUNT(*) - SUM(finishplace)';
      } else if ($scoring == 1) {
        $avg_expr = (1 + get_lane_count()).' * (COUNT(*) - 1) - (SUM(finishplace) - MAX(finishplace))';
      } else {
        $avg_expr = (1 + get_lane_count()).' - MIN(finishplace)';
      }
    } else {
      $avg_expr = $scoring_expr;
    }

    // GROUP BY makes each returned row a summary of the outcome for one racer in
    // one round.
    //
    // ORDER BY means rows are ordered from best showing to worst, which means
    // they're also ordered best to worst within any given group.
    $this->sql = ('SELECT Rounds.roundid,'
                  .' Rounds.classid as round_classid, Rounds.round, '
                  .' RegistrationInfo.racerid, firstname, lastname, carnumber, carname,'
                  // classid is the racer's 'native' class, not the round's class (e.g. Grand Finals)
                  .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank,'
                  // base is the number of heats in the round
                  .' COUNT(*) AS base, '
                  .$scoring_expr.' AS score, '.$avg_expr.' AS avg,'
                  .' MIN('.$raw_expr.') AS best, MAX('.$raw_expr.') AS worst'
                  .' FROM '.inner_join('RegistrationInfo', 'RaceChart',
                                       'RaceChart.racerid = RegistrationInfo.racerid',
                                       'Rounds',
                                       'RaceChart.roundid = Rounds.roundid',
                                       'Roster',
                                       'Roster.roundid = Rounds.roundid'
                                       .' AND Roster.racerid = RaceChart.racerid',
                                       'Classes',
                                       'RegistrationInfo.classid = Classes.classid',
                                       'Ranks',
                                       'RegistrationInfo.rankid = Ranks.rankid')
                  .' WHERE passedinspection = 1 AND exclude = 0'
                  .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)'
                  // Of these, only the roundid and racerid are meaningful
                  // grouping keys; each of the other values is uniquely
                  // determined by the racerid.
                  .' GROUP BY Rounds.roundid, RegistrationInfo.racerid,'
                  .'          RegistrationInfo.classid, Rounds.classid, class,'
                  .'          RegistrationInfo.rankid, rank, firstname, lastname, carnumber'
                  // lastname, firstname for reproducible ordering in the event of a tie
                  .' ORDER BY '.$scoring_expr.', lastname, firstname');
    $stmt = $db->query($this->sql);
    return $stmt->fetchAll(PDO::FETCH_ASSOC);
  }

  // compute_all_standings takes the $result_summary (representing standings for
  // each racer in each individual round, merged together into one aggregate list)
  // and computes all the other categories of standings.
  //
  // $ladders = compute_all_standings()
  //
  // $ladders maps each category key to a ladder of finishers for that category.
  //
  // compute_all_standings also annotates each entry of $this->result_summary
  // with an outcome: field.  The outcome value is a map from standings key to
  // ladder index in the ladder for the same key.  This is, for each racer's
  // appearance in a racing round, that appearance gets marked up to indicate
  // the racer's standing(s) determined by that appearance.
  private function compute_all_standings($for_awards = false) {
    $trackers = array();  // An array of category key => StandingsTracker

    // $add_outcome records a category place in the annotation entry in $parallel.
    $add_outcome = function($key, &$entry) use (&$trackers) {
      if (!isset($trackers[$key])) {
        $trackers[$key] = new StandingsTracker();
      }
      $place = $trackers[$key]->AddEntry($entry['score'], $entry['racerid']);
      $entry['outcome'][$key] = $place;
    };

    foreach ($this->result_summary as &$row) {
      $row['outcome'] = array();

      $add_outcome(stkey_round($row['roundid']), $row);

      if (!isset($this->unfinished_rounds[$row['roundid']])) {
        $cl = &$this->class_details[$row['round_classid']];
        if (count($cl['rounds']) > 0 && $row['roundid'] == $cl['rounds'][0]['roundid']) {
          // Final round for this class, so this round result bears on the class standing
          // and beyond
          if ($this->use_subgroups) {
            $add_outcome(stkey_round_subgroup($row['roundid'], $row['rankid']), $row);
          }

          foreach ($cl['rolls_up_to'] as $r) {
            $r_cl = $this->class_details[$r];
            if (count($r_cl['rankids']) == 0 || in_array($row['rankid'], $r_cl['rankids'])) {
              $add_outcome(stkey_qual($r), $row);
            }
          }
          // This assigns a full-field result to every scoring round, without
          // regard to whether there are aggregate rounds.
          $add_outcome(stkey_full_field(), $row);
        } else {
          $row['nonfinal'] = true;
          if ($this->use_subgroups) {
            // For non-final rounds, we can compute a standing for the racer
            // within their subgroup within the current round only.
            $add_outcome(stkey_round_subgroup($row['roundid'], $row['rankid']), $row);
          }
        }
      }
    }

    $this->all_standings = array();
    $this->catalog_counts = array();
    foreach ($trackers as $key => &$tracker) {
      $this->all_standings[$key] = $tracker->result;
      $this->catalog_counts[$key] = $tracker->count();
    }
    foreach ($this->result_summary as &$row) {
      foreach ($row['outcome'] as $key => &$place) {
        if (count($this->all_standings[$key][$place]) > 1) {
          $place = 'T'.($place + 1);
        } else {
          $place = $place + 1;
        }
      }
    }
  }


  // Full-field standings might be determined by standings for some
  // aggregate class, or simply not calculated at all.
  function has_full_field_standings() {
    return $this->full_field_calc == 0;
  }

  function award_ladder_full_field() {
    if (isset($this->all_standings[stkey_full_field()])) {
      return $this->all_standings[stkey_full_field()];
    } else {
      return array();
    }
  }

  function award_ladder_class($classid) {
    $rounds = $this->class_details[$classid]['rounds'];
    if (count($rounds) > 0) {
      // Scoring (final) round is listed first
      $key = stkey_round($rounds[0]['roundid']);
    } else {
      $key = stkey_qual($classid);
    }
    if (isset($this->all_standings[$key])) {
      return $this->all_standings[$key];
    } else {
      return array();
    }
  }

  function award_ladder_subgroup($subgroupid) {
    $classid = $this->ranks[$subgroupid]['classid'];
    $cl = $this->class_details[$classid];
    if (count($cl['rounds']) > 0) {
      $key = stkey_round_subgroup($cl['rounds'][0]['roundid'], $subgroupid);
      return isset($this->all_standings[$key]) ? $this->all_standings[$key] : array();
    } else {
      return array();
    }
  }

  function subgroups($classid) {
    $rankids = array();
    foreach ($this->class_details[$classid]['subgroups'] as $subgroup) {
      $rankids[] = $subgroup['rankid'];
    }
    return $rankids;
  }

  private function write_standings_table_headers() {
    echo "<tr class='headers'>";
    echo "<th>Place</th>";
    echo "<th>Car Number</th>";
    echo "<th>Name</th>";
    if (show_car_name_in_standings()) {
      echo "<th>Car Name</th>";
    }
    if ($this->use_groups) {
      echo "<th class='col-group'>".group_label()."</th>";
      echo "<th class='col-ingroup'>In <span id='per-group-label'>".group_label()."</span></th>";
    }
    if ($this->use_subgroups) {
      echo "<th class='col-subgroup'>".subgroup_label()."</th>";
      echo "<th class='col-insubgroup'>In <span id='per-subgroup-label'>".subgroup_label()."</span></th>";
    }
    if ($this->has_full_field_standings()) {
      echo "<th class='col-insuper'>In ".supergroup_label()."</th>";
    }
    echo "<th class='col-inround'>In Round</th>";

    echo "<th>Heats</th>";

    if ($this->use_points) {
      echo "<th>Points (1st = ".get_lane_count().")</th>";
    } else {
      echo "<th>Average</th>";
    }
    echo "<th>Best</th>";
    echo "<th>Worst</th>";
    echo "</tr>\n";
  }

  ////////////////////////////////////////////////////////////////////////////
  //
  // <tr> is marked with keys for all the outcome members in the entry.
  //
  // The first (primary "place") <td> has separate divs for:
  //    ROUND standings for the round
  //    QUAL
  //    ROUND SUBGROUP
  //    FULL FIELD standings (if there is one)
  //
  // If $use_subgroups, then columns exist for:
  //    group name
  //    ingroup with CLASS standing for the racer's class
  //    subgroup name
  //    insubgroup with SUBGROUP standing for the racer's rank
  //
  // If supergroup, then
  //    td.col-insuper with the full field standing

  private function write_standings_table_rows() {
    $name_style = read_name_style();
    $show_car_name = show_car_name_in_standings();
    $time_format = get_finishtime_formatting_string();

    $i = 0;
    foreach ($this->result_summary as &$row) {
      ++$i;

      $roundid = $row['roundid'];

      // js/standings-kiosk.js looks for data-first attribute for scrolling.
      echo "<tr data-roundid='".$roundid."' data-rankid='".$row['rankid']."'"
      ." data-for-supergroup='".isset($p['supergroup'])."'"
      .($i == 1 ? " data-first='1'" : "");
      echo ' class="';
      foreach ($row['outcome'] as $key => $p) {
        echo $key.' ';
      }
      echo '"';
      echo ">";

      $p = &$row['outcome'];

      // First column:
      echo "<td class='col0'>";
      {
        // echo json_encode($p);  // DEBUG

        echo "<div class='inround'>";
        {
          echo $p[stkey_round($roundid)];
        }
        echo "</div>";

        if (isset($p[stkey_full_field()])) {
          echo "<div class='insuper'>";
          echo $p[stkey_full_field()];
          echo "</div>";
        }

        foreach ($p as $key => $place) {
          if (is_stkey_qual($key)) {
            echo "<div class=\"$key\">$place</div>";
          }
        }
      }
      echo "</td>";

      echo "<td>".$row['carnumber']."</td>";
      echo "<td>".htmlspecialchars(mangled_name($row, $name_style), ENT_QUOTES, 'UTF-8')."</td>";
      if ($show_car_name) {
        echo "<td>".$row['carname']."</td>";
      }

      // Racer's (native) class and place in class/group.  These columns only get shown
      // for non-aggregate racing rounds, so round_classid == classid
      if ($this->use_groups) {
        $key = stkey_round($row['roundid']);  // Racer's classid

        echo "<td class='col-group'>".htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8')."</td>";
        echo "<td class='col-ingroup'>";
        if (isset($p[$key])) {
          echo $p[$key];
        }
        echo "</td>";
      }

      // Rank/subgroup and place in rank/subgroup
      if ($this->use_subgroups) {
        echo "<td class='col-subgroup'>".htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8')."</td>";
        $key = stkey_round_subgroup($row['roundid'], $row['rankid']);
        echo "<td class='col-insubgroup'>";
        if (isset($p[$key])) {
          echo $p[$key];
        }
        echo "</td>";
      }

      // Place in pack
      if ($this->has_full_field_standings()) {
        echo "<td class='col-insuper'>";
        if (isset($p[stkey_full_field()])) {
          echo $p[stkey_full_field()];
        }
        echo "</td>";
      }
      echo "<td class='col-inround'>".$p[stkey_round($roundid)]."</td>";

      echo "<td>".$row['base']."</td>";

      if ($this->use_points) {
        echo "<td>".$row['avg']."</td>";
        echo "<td>".ordinal($row['best'])."</td>";
        echo "<td>".ordinal($row['worst'])."</td>";
      } else {
        echo "<td>".sprintf($time_format, $row['avg'])."</td>";
        echo "<td>".sprintf($time_format, $row['best'])."</td>";
        echo "<td>".sprintf($time_format, $row['worst'])."</td>";
      }
      echo "</tr>\n";
    }
  }

  public function write_standings_table() {
    $this->write_standings_table_headers();
    $this->write_standings_table_rows();
  }

  function highest_round($classid) {
    $rounds = $this->class_details[$classid]['rounds'];
    for ($i = 0; $i < count($rounds); ++$i) {
      if (!isset($this->unfinished_rounds[$rounds[$i]['roundid']])) {
        return $rounds[$i]['roundid'];
      }
    }

    return null;
  }

  public function class_key($classid) {
    $roundid = $this->highest_round($classid);
    if ($roundid) {
      return stkey_round($roundid);
    }
    if (!empty($this->class_details[$classid]['constituents'])) {
      return stkey_qual($classid);
    }
  }

  public function rank_key($rankid) {
    $classid = $this->ranks[$rankid]['classid'];
    $roundid = $this->highest_round($classid);
    if ($roundid) {
      return stkey_round_subgroup($roundid, $rankid);
    }
  }
  
  // Presentation determines:
  // (1) What to show in the first column
  // (2) Which other columns to show:
  // +--------------+-------+-------+---------+-------+--------------+-----+
  // |              | ff    |class  |subgroup |round  |round-subgroup|qual |
  // +--------------+-------+-------+---------+-------+--------------+-----+
  // | first column |insuper|inround|inround  |inround|  inround     | --  |
  // +--------------+-------+-------+---------+-------+--------------+-----+
  // | group?       | YES   | YES   |         |       |              |     |
  // +--------------+-------+-------+---------+-------+--------------+-----+
  // | ingroup?     | YES   | YES   | YES     |       |              | YES |
  // +--------------+-------+-------+---------+-------+--------------+-----+
  // | subgroup?    | YES   | YES   |         | YES   |              | YES |
  // +--------------+-------+-------+---------+-------+--------------+-----+
  // | insubgroup?  | YES   | YES   | YES     | YES   | YES          | YES |
  // +--------------+-------+-------+---------+-------+--------------+-----+
  // | insuper?     |       |       | YES     |       |              |     |
  // +--------------+-------+-------+---------+-------+--------------+-----+
  // | inround?     |       |       |         |       |              |     |
  // +--------------+-------+-------+---------+-------+--------------+-----+
  public function standings_catalog() {
    $catalog = array();

    if ($this->has_full_field_standings()) {
      $catalog[] = array('name' => supergroup_label(),
                         'key' => stkey_full_field(),
                         'presentation' => 'ff');
    }

    $rankseq_index = 0;
    foreach ($this->classseq as $classid) {
      $cl = $this->class_details[$classid];
      $details = $this->class_details[$classid];
      if (count($details['rounds']) > 0) {
        $rounds = $details['rounds'];
        // The final round for a class is listed first.
        // Skip any unfinished rounds to get to the last completed round.
        do {
          $sr = array_shift($rounds);
          if (!is_null($sr)) {
            $scoring_roundid = $sr['roundid'];
          } else {
            $scoring_roundid = null;
          }
        } while (!is_null($scoring_roundid) && isset($this->unfinished_rounds[$scoring_roundid]));

        $this->class_keys[$classid] = stkey_round($scoring_roundid);

        if ($this->use_groups) {
          $catalog[] = array('name' => $cl['class'],
                             'key' => stkey_round($scoring_roundid),
                             'presentation' => 'class');
        }

        for (; $rankseq_index < count($this->rankseq); ++$rankseq_index) {
          $rank = $this->ranks[$this->rankseq[$rankseq_index]];
          if ($rank['classid'] != $classid) {
            break;
          }

          if ($this->use_subgroups) {
            $catalog[] = array('name' => $cl['class'].' / '.$rank['rank'],
                               'key' => stkey_round_subgroup($scoring_roundid, $rank['rankid']),
                               'presentation' => 'subgroup');
          }
        }

        // Loop through the non-final rounds for the class
        foreach ($rounds as $round) {
          if ($this->use_groups) {
            $round_name =  $cl['class'].', Round '.$round['round'];
          } else {
            $round_name = 'Round '.$round['round'];
          }
          $catalog[] = array('name' => $round_name,
                             'key' => stkey_round($round['roundid']),
                             'presentation' => 'round');
          if ($this->use_subgroups) {
            foreach ($this->rankseq as $rankid) {
              $rank = $this->ranks[$rankid];
              if ($rank['classid'] == $classid) {
                $catalog[] = array('name' => $round_name.' / '.$rank['rank'],
                                   'key' => stkey_round_subgroup(
                                     $round['roundid'], $rank['rankid']),
                                   'presentation' => 'round-subgroup');
              }
            }
          }
        }

        if (count($cl['constituents']) > 0) {
          $catalog[] = array('name' => $cl['class'].' Qualifying Rounds',
                             'key' => stkey_qual($classid),
                             'presentation' => 'qual');
        }
      } else {
        // Nonracing aggregate class
        $this->class_keys[$classid] = stkey_qual($classid);
        $catalog[] = array('name' => $cl['class'],
                           'key' => stkey_qual($classid),
                           'presentation' => 'qual');
      }

    }

    return $catalog;
  }

  function debug_summary() {
    return array('class_details' => $this->class_details,
                 'unfinished_rounds' => $this->unfinished_rounds,
                 'sql' => $this->sql,
                 'result_summary' => $this->result_summary,
                 'all_standings' => $this->all_standings);
  }
}

// Returns roundids for unscheduled or incompletely-raced rounds
function unfinished_rounds() {
  global $db;
  // GPRM databases may have RaceChart entries with null racerid for byes; these
  // do not mark an unfinished heat.  A round is "unfinished" if it has no heats
  // scheduled, or if there are scheduled heats that have no results.
  $stmt = $db->query('SELECT roundid, classid FROM Rounds'
                     .' WHERE NOT EXISTS(SELECT 1 FROM RaceChart'
                     .'           WHERE RaceChart.roundid = Rounds.roundid)'
                     .' OR EXISTS(SELECT 1 FROM RaceChart'
                     .'           WHERE RaceChart.roundid = Rounds.roundid'
                     .'             AND racerid IS NOT NULL'
                     .'             AND finishtime IS NULL AND finishplace IS NULL)');
  $result = array();
  foreach ($stmt as $row) {
    $result[$row[0]] = $row[1];
  }
  return $result;
}

function show_car_name_in_standings() {
  return read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                           .' WHERE carname IS NOT NULL AND carname > \'\'');
}

class StandingsTracker {
  private $last_score = -1;
  // The number of recorded scores for this tracker.  This lets us "skip ahead"
  // when there are ties.
  private $count = 0;
  public function count() { return $this->count; }

  // $result[$i] gives the racerids for ($i+1)'th place
  // E.g., [[11], [1], [21, 41], [], [31]] to indicate a tie for 3rd place.
  // (There are no 4th place racers in this example; racerid 31 is in 5th
  // place.)
  public $result = array();

  // Called in order by score, that is, $score is always no less than all
  // previous scores in all previous calls for this tracker.  Returns the
  // zero-based numeric place, with no indication for ties.
  public function AddEntry($score, $racerid) {
    ++$this->count;
    if ($score <= $this->last_score) {
      // A tie!
      $place = count($this->result) - 1;  // Index of last populated "place" in the results
      $this->result[$place][] = $racerid;
      // The tricky bit here is that only upon the arrival of the second racer
      // do we find out that there was a tie; we need to be able to go back to
      // the first racer and update.  That's why we make a second pass to mark
      // ties, rather than trying to do it on the fly here.
    } else {
      while (count($this->result) < $this->count - 1) {
        $this->result[] = array();  // Empty slots for ties
      }
      $this->result[] = array($racerid);
      $place = $this->count - 1;
    }

    $this->last_score = $score;
    return $place;
  }
}
?>
