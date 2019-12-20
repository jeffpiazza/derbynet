<?php

require_once('inc/data.inc');
require_once('inc/name-mangler.inc');
require_once('inc/schema_version.inc');
require_once('inc/rounds.inc');

class ScheduleDocument extends PrintableSummaryDocument {
  function name() { return "Schedule"; }

  function get_available_options() {
    return array();
  }

  function StartDocument() {
    $this->initialize_pdf('P', 'in', 'Letter');
  }

  function Byes($n) {
    while ($n > 0) {
      $this->pdf->SetFont('Times', '', 10);
      $this->pdf->Cell(1.00, 0.5, 'Bye', 1, 0, "C");
      --$n;
    }
  }

  function Newline() {
    $this->pdf->Ln();
    if ($this->pdf->GetY() > 10.0) {
      $this->pdf->AddPage();
    }
  }

  function Draw() {
    global $db;
    $this->pdf->AddPage();

    $nlanes = get_lane_count_from_results();
    $use_master_sched = use_master_sched();
    $name_style = read_raceinfo('name-style', FULL_NAME);

    // array of {round, groupid, groupname},
    //  but groupid and groupname are really a roundid and round name
    $groups = all_schedule_groups();

    $sql = 'SELECT'
    .' Classes.class, round, heat, lane, finishtime, resultid, completed,'
    .' RegistrationInfo.racerid,'
    .($use_master_sched ? 'round' : 'Rounds.roundid').' as racinggroup,'
    .($use_master_sched ? 'masterheat' : 'heat').' as seq,'
    .' RegistrationInfo.carnumber, RegistrationInfo.firstname, RegistrationInfo.lastname,'
    .' Classes.classid, Rounds.roundid, RaceChart.racerid'
    .' FROM '.inner_join('RaceChart', 'RegistrationInfo', 
                         'RegistrationInfo.racerid = RaceChart.racerid',
                         'Roster', 'Roster.racerid = RegistrationInfo.Racerid',
                         'Rounds', 'Rounds.roundid = Roster.roundid',
                         'Classes', 'Rounds.classid = Classes.classid')
    .' WHERE Rounds.roundid = RaceChart.roundid'
    .' ORDER BY '
    .($use_master_sched
      ? 'round, masterheat, lane'
      : ((schema_version() >= 2 ? 'Classes.sortorder, ' : '')
         .'class, round, heat, lane'));

    $stmt = $db->query($sql);
    
    // Each time through the "groups" loop (which is really rounds), $rs is
    // expected to be populated already.
    $rs = $stmt->fetch(PDO::FETCH_ASSOC);

    $lane = $nlanes;
    foreach ($groups as $group) {
      $roundid = $group['groupid'];
      $round_name = $group['groupname'];
      $this->Newline();
      $this->pdf->SetFont('Arial', 'B', 15);
      $this->pdf->Cell(7.0, 0.5, convert($round_name), /* border */ 0, /* ln */0, "C");

      $seq = -1;
      while ($rs and $rs['racinggroup'] == $roundid) {
        if ($seq <> $rs['seq']) {
          $this->Byes($nlanes - $lane);
          $seq = $rs['seq'];
          $lane = 0;
          $this->Newline();
          $this->pdf->SetFont('Times', '', 15);
          $this->pdf->Cell(1.25, 0.5, 'Heat '.$seq, /* border */ 0, /* ln */ 0, "R");
        }
        $new_lane = $rs['lane'];
        $this->Byes($new_lane - $lane - 1);

        $x = $this->pdf->GetX();
        $y = $this->pdf->GetY();

        $this->pdf->SetXY($x, $y + 0.165);
        $this->pdf->SetFont('Times', '', '8');
        $this->pdf->Cell(1.00, 0.5, convert($rs['firstname'].' '.$rs['lastname']), 0, 0, "C");
        
        $this->pdf->SetXY($x, $y);        
        $this->pdf->SetFont('Times', '', 15);
        $this->pdf->Cell(1.00, 0.5, $rs['carnumber'],
                         /* border */ 1, /* ln */ 0, "C");

        $lane = $new_lane;
        $rs = $stmt->fetch(PDO::FETCH_ASSOC);
      }
      $this->Byes($nlanes - $lane);
      $lane = $nlanes;
    }
  }
}
?>