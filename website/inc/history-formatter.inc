<?php

$debug_history_entries = false;

function EmitErrorRow($msg) {
  global $debug_history_entries;
  if ($debug_history_entries) {
    echo "<tr><td colspan='3' style='background: #ffaaaa; font-weight: bold;'>$msg</td></tr>";
  }
}

class SimulatedSqliteTable {
  public $_entries = array();
  private $_idcol;

  public function __construct($idcol) {
    $this->_idcol = $idcol;
  }

  public function &Insert($v) {
    if (isset($v[$this->_idcol])) {
      EmitErrorRow("Inappropriate Insert ".json_encode($v));
    }
    $index = 1 + count($this->_entries);
    $v[$this->_idcol] = $index;
    $this->_entries[] = &$v;
    return $v;
  }
  public function &InsertWithId($v) {
    if (!isset($v[$this->_idcol])) {
      EmitErrorRow("Inappropriate InsertWithId ".json_encode($v));
    }
    if (false && !array_is_list($this->_entries)) {  // TODO
      EmitErrorRow("InsertWithId notices we're not a list: ".$this->_idcol);
      echo "<tr><td colspan='3' style='background: #ffff99;'>InsertWithId after appending:<br/>"
        .$this->DumpEntries()
        ."</td></tr>";
    }
    $index = intval($v[$this->_idcol]);
    if (intval($index - 1) < 0) {
      EmitErrorRow("InsertWithId with bad id column ".json_encode($v));
    }
    if (1 + count($this->_entries) <= $index) {
      while (1 + count($this->_entries) < $index) {
        $this->_entries[] = null;
      }
      $this->_entries[] = &$v;
      if (false && !array_is_list($this->_entries)) {
        echo "<tr><td colspan='3' style='background: #ffff99;'>InsertWithId after appending:<br/>"
          .$this->DumpEntries()
          ."</td></tr>";  // TODO
      }
    } else {
      $this->_entries[$index - 1] = &$v;
    }
    return $v;
  }

  public function &Get($id) {
    if ($id > 0 && $id <= count($this->_entries)) {
      return $this->_entries[$id - 1];
    }
    $empty = null;
    return $empty;
  }
  public function &GetOrError($id) {
    $id = intval($id);
    if ($id > 0) {
      $v = &$this->Get($id);
    }
    if (!$v) {
      EmitErrorRow($this->_idcol." not found: $id");
    }
    return $v;
  }

  public function &Find($col, $val) {
    $count = count($this->_entries);
    for ($i = 0; $i < $count; ++$i) {
      if ($this->_entries[$i] &&
          $this->_entries[$i][$col] == $val) {
        return $this->_entries[$i];
      }
    }
    $empty = null;
    return $empty;
  }

  public function &Find2($col1, $val1, $col2, $val2) {
    $count = count($this->_entries);
    for ($i = 0; $i < $count; ++$i) {
      if ($this->_entries[$i] &&
          $this->_entries[$i][$col1] == $val1 &&
          $this->_entries[$i][$col2] == $val2) {
        return $this->_entries[$i];
      }
    }
    $empty = null;
    return $empty;
  }

  public function &FindOrInsert($col, $val) {
    $f = $this->Find($col, $val);
    if ($f) {
      return $f;
    }
    return $this->Insert(array($col => $val));
  }

  public function &FindOrInsert2($col1, $val1, $col2, $val2) {
    $f = $this->Find2($col1, $val1, $col2, $val2);
    if ($f) {
      return $f;
    }
    return $this->Insert(array($col1 => $val1, $col2 => $val2));
  }

  public function Delete($id) {
    $cnt = count($this->_entries);
    if ($id > 0 && $id == $cnt) {
      array_pop($this->_entries);
    } else if ($id > 0 && $id < $cnt) {
      $this->_entries[$id - 1] = null;
    } else {
      EmitErrorRow("Delete can't find $id (".$this->_idcol.")");
    }
  }

  public function MakeEmpty() {
    $this->_entries = array();
  }

  public function DumpEntries() {
    return "<pre style='text-align: left;'>\n"
      .json_encode($this->_entries, JSON_PRETTY_PRINT)
      ."</pre>";
  }

  public function DumpEntriesAsRow() {
    echo "<tr><td colspan='3'>".$this->DumpEntries()."</td></tr>";
  }
}

class HistoryActionFormatter {
  public function __construct() {
    $this->_racers = new SimulatedSqliteTable('racerid');
    $this->_classes = new SimulatedSqliteTable('classid');
    $this->_ranks = new SimulatedSqliteTable('rankid');
    $this->_partitions = new SimulatedSqliteTable('partitionid');
    $this->_rounds = new SimulatedSqliteTable('roundid');

    $this->_settings = array();
    $this->_roundid = null;
    $this->_heat = null;
  }

  private function DumpModel() {
    return "<pre style='text-align: left;'>\n"
      .json_encode(array('classes' => $this->_classes->_entries,
                         'ranks' => $this->_ranks->_entries,
                         'rounds' => $this->_rounds->_entries,
                         'partitions' => $this->_partitions->_entries,
                         'racers' => $this->_racers->_entries),
                   JSON_PRETTY_PRINT)
      ."</pre>";
  }

  // $action_string is a json-encoded string of the $_POST received by
  // action.php
  public function SetAction($action_string) {
    $this->_action_string = htmlspecialchars($action_string, ENT_QUOTES, 'UTF-8');
    $action = json_decode($action_string, true);
    foreach ($action as $key => $val) {
      if (is_string($val)) {
        $action[$key] = htmlspecialchars($action[$key], ENT_QUOTES, 'UTF-8');
      }
    }
    $this->_action = $action;

    // TODO
    if (!array_is_list($this->_racers->_entries)) {
      EmitErrorRow("Racers are not a list!");
      echo "<tr><td colspan='3'>".$this->DumpModel()."</td></tr>";
    } else if (!array_is_list($this->_classes->_entries)) {
      EmitErrorRow("Classes are not a list!");
      echo "<tr><td colspan='3'>".$this->DumpModel()."</td></tr>";
    } else if (!array_is_list($this->_ranks->_entries)) {
      EmitErrorRow("Ranks are not a list!");
      echo "<tr><td colspan='3'>".$this->DumpModel()."</td></tr>";
    } else if (!array_is_list($this->_partitions->_entries)) {
      EmitErrorRow("Partitions are not a list!");
      echo "<tr><td colspan='3'>".$this->DumpModel()."</td></tr>";
    } else if (!array_is_list($this->_rounds->_entries)) {
      EmitErrorRow("Rounds are not a list!");
      echo "<tr><td colspan='3'>".$this->DumpModel()."</td></tr>";
    }

    if (isset($this->_racers->_entries[-1]) || isset($this->_racers->_entries["-1"])) {
      EmitErrorRow("Racers are broken (racerid -1)!");
      echo "<tr><td colspan='3'>".$this->DumpModel()."</td></tr>";
    }

    if (isset($this->_rounds->_entries[-1]) || isset($this->_rounds->_entries["-1"])) {
      EmitErrorRow("Rounds are broken (roundid -1)!");
      echo "<tr><td colspan='3'>".$this->DumpModel()."</td></tr>";

    }
  }

  public function CurrentHeatString() {
    if ($this->ShouldShowCurrentHeat() &&
        isset($this->_roundid) &&
        isset($this->_heat)) {
      return $this->HeatString($this->_roundid, $this->_heat);
    }
    return "";
  }
  
  private function RacerString($racer) {
    $s = "<b>$racer[firstname] $racer[lastname]</b>";
    if (isset($racer['carnumber'])) {
      $s .= ", $racer[carnumber]";
    }
    if (isset($racer['rankid'])) {
      $r = $this->_ranks->GetOrError($racer['rankid']);
      if ($r) {
        $s .= ', '.$r['rank'];
      }
    } else if (isset($racer['classid'])) {
      $c = $this->_classes->GetOrError($racer['classid']);
      if ($c) {
        $s .= ', '.$c['class'];
      }
    } else if (isset($racer['partition'])) {
      $s .= ', '.$racer['partition'];
    }
    return $s;
  }

  private function RoundString($roundid) {
    $round = $this->_rounds->GetOrError($roundid);
    if (!$round) {
      return "Roundid <b>$roundid</b>";
    }
    $c = $this->_classes->GetOrError($round['classid']);
    if (!$c) {
      $c = array('class' => "<b>No class $round[classid]</b>");
    }
    $rno = $round['round'] == 1 ? "" : ", round $round[round]";
    return "$c[class]$rno";
  }
  
  private function HeatString($roundid, $heat) {
    return $this->RoundString($roundid).", heat $heat";
  }

  private function EffectOnlyRow() {
    global $debug_history_entries;
    if ($debug_history_entries) {
      echo "<tr><td colspan='3' style='background: #dddddd;'>Processed <b>".$this->_action_string."</b></td></tr>\n";
    }
  }

  private function PartitionLabel($lc = false) {
    $label = @$this->_settings['partition-label'];
    if (!$label) $label = 'Den';
    return $lc ? strtolower($label) : $label;
  }

  public function Process() {
    global $debug_history_entries;
    switch ($this->_action['action']) {
    case 'award.*delete':
    case 'award.adhoc':
    case 'award.delete':
    case 'award.edit':
    case 'award.import':
    case 'award.order':
    case 'award.present':
    case 'award.winner':
    case 'award.xbs':
      return "";
    case 'class.*create':
      $this->_classes->InsertWithId(array('classid' => $this->_action['classid'],
                                          'class' => $this->_action['class']));
      return $this->EffectOnlyRow();
    case 'class.*delete':
      $this->_classes->Delete($this->_action['classid']);
      return $this->EffectOnlyRow();
    case 'class.add':
      return "Add class ".$this->_action['name'];
    case 'class.delete': {
      $c = $this->_classes->GetOrError($this->_action['classid']);
      return "Delete class $c[class]";
    }
    case 'class.edit': {
      $c = &$this->_classes->Get($this->_action['classid']);
      if (isset($this->_action['name'])) {
        $c['class'] = $this->_action['name'];
      }
      if (false) {
        $c = &$this->_classes->GetOrError($this->_action['classid']);
        return "Edit class $c[class]";
      }
      return $this->EffectOnlyRow();
    }
    case 'class.order':
      return "Re-order racing groups";
    case 'database.execute':
      if ($this->_action['script'] == 'update-schema') {
        return 'Update Database Schema';
      }
      break;
    case 'heat.*advance':
      $this->_roundid = $this->_action['roundid'];
      $this->_heat = $this->_action['heat'];
      return $this->EffectOnlyRow();
    case 'heat.select':
      if (isset($this->_action['now_racing']) && !$this->_action['now_racing']) {
        return "Stop racing";
      } else if (isset($this->_action['roundid'])) {
        if (isset($this->_action['heat'])) {
          return "Heat select: ".$this->HeatString($this->_action['roundid'], $this->_action['heat']);
        } else {
          return "Start racing: ".$this->RoundString($this->_action['roundid']);
        }
      } else if (isset($this->_action['now_racing'])) {
        return "Start racing";
      } else {
        return "<div style='font-weight: bold;'>".$this->_action_string."</div>";  // TODO
      }
    case 'kiosk.assign':  return "";
    case 'partition.*create':
      $this->_partitions->InsertWithId(array('partitionid' => $this->_action['partitionid'],
                                             'name' => $this->_action['name'],
                                             'rankid' => $this->_action['rankid']));
      return $this->EffectOnlyRow();
    case 'partition.*structure': {
      $this->_classes->MakeEmpty();
      $this->_ranks->MakeEmpty();
      $this->_rounds->MakeEmpty();
      foreach ($this->_action['partitions'] as $part) {
        $partition = &$this->_partitions->Get($part['partitionid']);
        if (!$partition) {
          EmitErrorRow("Partition structure failure #0: ".json_encode($part));
        }
        $partition['rankid'] = $part['rankid'];
        $rank = &$this->_ranks->Get($part['rankid']);
        if (!$rank) {
          $rank = &$this->_ranks->InsertWithId(array('rankid' => $part['rankid']));
          // echo "<br/><b>Partition structure failure #1 (rankid)</b><br/>";
        }
        $rank['rank'] = $partition['name'];
        $rank['classid'] = $part['classid'];
        $c = &$this->_classes->Get($part['classid']);
        if (!$c) {
          $c = &$this->_classes->InsertWithId(array('classid' => $part['classid']));
          // echo "<br/><b>Partition structure failure #2 (classid)</b><br/>";
        }
        if (isset($this->_action['class'])) {
          $c['class'] = $this->_action['class'];
        } else {
          $c['class'] = $partition['name'];
        }
        $round = &$this->_rounds->Get($part['roundid']);
        if (!$round) {
          $round = &$this->_rounds->InsertWithId(array('roundid' => $part['roundid'], 'round' => 1));
          // echo "<br/><b>Partition structure failure #3 (roundid)</b><br/>";
        }
        $round['classid'] = $part['classid'];
      }

      foreach ($this->_racers->_entries as &$racer) {
        $partid = $racer['partitionid'];
        $p = $this->_partitions->Get($partid);
        $racer['rankid'] = $p['rankid'];
        // $rank = $this->_ranks->Get($p['rankid']);
        // $racer['classid'] = $rank['classid'];
      }
      
    }
      return $this->EffectOnlyRow();
    case 'partition.add':
      return "Add ".$this->PartitionLabel(true)." ".$this->_action['name'];
    case 'partition.apply-rule': {
      $rule = $this->_action['rule'];
      if ($rule == 'by-partition') {
        $rule = 'Race each '.$this->PartitionLabel(true).' as a group.';
      } else if ($rule == 'one-group') {
        $rule = 'Race as one big group.';
      } else {
        $rule = 'Custom racing groups.';
      }
      return 'Racing groups: '.$rule;
    }
    case 'partition.delete': {
      $p = $this->_partitions->Get($this->_action['partitionid']);
      $this->_partitions->Delete($this->_action['partitionid']);
      return /* TODO */"[DID IT] Delete partition $p[name]";
    }
    case 'partition.edit': {
      $p = &$this->_partitions->Get($this->_action['partitionid']);
      if (isset($this->_action['name'])) {
        $old = $p['name'];
        $p['name'] = $this->_action['name'];
        return /*TODO*/"[DID IT] Rename ".$this->PartitionLabel(true)." $old to $p[name]";
      }
      break;
    }
    case 'partition.move': {
      // {"action":"partition.move","div_id":"2","group_field":"classid","group_id":"1"}
      $p = $this->_partitions->GetOrError($this->_action['div_id']);
      if ($this->_action['group_id'] < 0) {
        return "Move ".$this->PartitionLabel(true)." $p[name] to its own racing group";
      }
      $c = $this->_classes->GetOrError($this->_action['group_id']);
      return "Move ".$this->PartitionLabel(true)." $p[name] to racing group $c[class]";
    }
    case 'partition.order': return "";
    case 'photo.assign': return "";
    case 'photo.crop': return "";
    case 'photo.delete': return "";
    case 'photo.rotate': return "";
    case 'photo.upload': return "";
    case 'racer.*create':
      $this->_racers->InsertWithId(array('racerid' => $this->_action['racerid'],
                                         'firstname' => $this->_action['firstname'],
                                         'lastname' => $this->_action['lastname'],
                                         'carnumber' => $this->_action['carnumber'],
                                         'carname' => $this->_action['carname'],
                                         'partitionid' => $this->_action['partitionid'],
                                         'rankid' => $this->_action['rankid'],
                                         'classid' => $this->_action['classid']));
      return $this->EffectOnlyRow();
    case 'racer.*delete':
      $this->_racers->Delete($this->_action['racerid']);
      return $this->EffectOnlyRow();
    case 'racer.*renumber': {
      foreach ($this->_action['results'] as $racerid => $carno) {
        $racer = &$this->_racers->GetOrError($racerid);
        $racer['carnumber'] = intval($carno);
      }
    }
      return $this->EffectOnlyRow();
      // case 'racer.add':  See racer.import
    case 'racer.bulk':
      switch ($this->_action['what']) {
      case 'checkin':
        if (@$this->_action['value']) {
          return "Check in racers in bulk";
        } else {
          return "Cancel check-ins in bulk";
        }
      case 'number':
        return "Renumber racers";
      case 'eligibility':
        return "Make racers ".($this->_action['value'] ? "eligible" : "ineligible")." in bulk";
      }

    case 'racer.delete':
      return "Delete racer ".$this->RacerString($this->_racers->Get($this->_action['racer']));
    case 'racer.edit': {
      $racer = &$this->_racers->Get($this->_action['racer']);
      foreach ($this->_action as $key => $value) {
        if ($key != 'action' && $key != 'racerid') {
          if ($key == 'carno') {
            $racer['carnumber'] = $value;
          } else {
            $racer[$key] = $value;
          }
        }
      }
      if (false) {
        $racerid_col = isset($this->_action['racerid']) ? 'racerid' : 'racer';
        $racer = &$this->_racers->GetOrError($this->_action[$racerid_col]);
        return "Edit racer ".$this->RacerString($racer).": ".$this->_action_string;  // TODO
      }
      return $this->EffectOnlyRow();
    }
    case 'racer.add':
    case 'racer.import':
      if (isset($this->_action['carno'])) {
        $this->_action['carnumber'] = $this->_action['carno'];
      }
      // TODO There will be a racer.*create that follows with the name already straightened out, so
      // we could just report that, instead, possibly just recording the verb ahead of time.
      if (isset($this->_action['first-last'])) {
        $this->_action['first-last'] = trim($this->_action['first-last']);
        $comma = strpos($this->_action['first-last'], ',');
        if ($comma !== false) {
          $this->_action['firstname'] = trim(substr($this->_action['first-last'], $comma + 1));
          $this->_action['lastname'] = trim(substr($this->_action['first-last'], 0, $comma));
        } else {
          $space = strrpos($this->_action['first-last'], ' ');
          if ($space !== false) {
            $this->_action['firstname'] = trim(substr($this->_action['first-last'], 0, $space));
            $this->_action['lastname'] = trim(substr($this->_action['first-last'], $space + 1));
          } else {
            $this->_action['firstname'] = '';
            $this->_action['lastname'] = $first_last;
          }
        }
      }
      return ($this->_action['action'] == 'racer.add' ? "Add" : "Import")
                                       ." racer ".$this->RacerString($this->_action);
    case 'racer.pass': {
      // {"action":"racer.pass","racer":1,"value":1}
      if (isset($this->_action['racer'])) {
        $racer = $this->_racers->Get($this->_action['racer']);
      } else if (isset($this->_action['barcode'])) {
        if (str_starts_with($this->_action['barcode'], 'PWDid')) {
          $racer = $this->_racers->Get(substr($this->_action['barcode'], 5));
        } else {
          $racer = $this->_racers->Find('carnumber', substr($this->_action['barcode'], 3));
        }
      }

      $verb = (isset($this->_action['value']) && !$this->_action['value'])
            ? "<b>Cancel</b> check-in of " : "Check in ";

      if (!$racer) {
        EmitErrorRow("$verb unmatched: <b>".json_encode($this->_action)."</b>");
        return "";
      } else {
        return "$verb ".$this->RacerString($racer);
      }
    }

    case 'rank.*create':
      $this->_ranks->InsertWithId(array('rankid' => $this->_action['rankid'],
                                        'rank' => $this->_action['rank'],
                                        'classid' => $this->_action['classid']));
      return $this->EffectOnlyRow();
    case 'rank.*delete':
      $this->_ranks->Delete($this->_action['rankid']);
      return $this->EffectOnlyRow();
    case 'rank.add':
      return "Add ".$this->PartitionLabel(true)." ".$this->_action['name'];
    case 'rank.delete': {
      $r = $this->_ranks->GetOrError($this->_action['rankid']);
      return "Delete ".$this->PartitionLabel(true)." $r[rank]";
    }
    case 'rank.edit':      // TODO Effect?
      return 'Rank.edit '.$this->_action_string;
    case 'rank.move': {  // {"action":"rank.move","rankid":"4","classid":"4"}
      $rank = $this->_ranks->GetOrError($this->_action['rankid']);
      if ($this->_action['classid'] < 0) {
        return "Move ".$this->PartitionLabel(true)." $rank[rank] into its own racing group";
      } else {
        $c = $this->_classes->GetOrError($this->_action['classid']);
        return "Move ".$this->PartitionLabel(true)." $rank[rank] into racing group $c[class]";
      }
    }
    case 'rank.order':
      return 'Re-order subgroups';
    case 'replay.message':  return "";
    case 'replay.test':  return "";
    case 'round.*create':
      $this->_rounds->InsertWithId(array('roundid' => $this->_action['roundid'],
                                         'classid' => $this->_action['classid'],
                                         'round' => $this->_action['round']));
      return $this->EffectOnlyRow();
    case 'scene.apply':  return "";
    case 'schedule.generate':
      return "Generate schedule for ".$this->RoundString($this->_action['roundid']);
    case 'schedule.reschedule':
      if (@$this->_action['dry-run']) {
        return "";
      }
      break;
    case 'settings.write':
      $msg = 'Write setting(s):';
      foreach ($this->_action as $key => $val) {
        if ($key != 'action' && !str_ends_with($key, '-checkbox')) {
          $this->_settings[$key] = $val;
          $msg .= ' <b>'.$key.'</b>: '.json_encode($val, /*TODO JSON_HEX_TAG | JSON_HEX_AMP |*/ JSON_UNESCAPED_SLASHES);
        }
      }
      return $msg;

    case 'timer.*overdue':
      return "Timer contact lost.";
    case 'timer-message':
      switch ($this->_action['message']) {
      case 'HELLO': return 'Timer makes contact';
      case 'IDENTIFIED': return 'Timer is identified';
      case 'FLAGS': return '';
      case 'STARTED': return 'Heat started';
      case 'FINISHED': $msg = "Heat finished:";
        // TODO
        $any_times = false;
        foreach ($this->_action as $key => $val) {
          if (str_starts_with($key, 'lane')) {
            $msg .= ' '.$key.': '.$val.'s';
            $any_times = true;
          } else if (!$any_times && str_starts_with($key, 'place')) {
            $msg .= ' lane'.substr($key, 5).": #$val";
          }
        }
        return $msg;
      }
    }

    if ($debug_history_entries) {
      return "<div style='background: #ffaaaa; font-weight: bold;'>Unimplemented: ".$this->_action_string."</div>";
    }
    return "";
  }

  private function ShouldShowCurrentHeat() {
    if ($this->_action['action'] == 'timer-message') {
      return array_search($this->_action['message'], array('STARTED', 'FINISHED', 'MALFUNCTION')) !== false;
    }
    return array_search($this->_action['action'], array('result.write')) !== false;
  }
}

?>
