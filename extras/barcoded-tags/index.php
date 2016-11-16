<?php
// This script renders barcoded car tags, each 2 3/4" by 8 1/2", four per page.
// Each tag has a three-digit number, with the first digit corresponding to the
// Cub Scout rank (e.g., 1xx tags are for Tigers, 2xx tags are Wolves, etc.)  A
// JPG image also identifies the rank on each tag.
//
// The printed bar code decodes as a six-character string: "PWD" plus the three
// digits of the tag.
//
// As written, the script renders 32 tags for each of ranks 1-4, plus 16 tags
// for rank 5 ("Friends").  Update the sequential_tags() function to change
// this.
//
// Instead of printing sequential tags, it's possible to just print ad hoc tags
// (for tags that got lost from your last event, say).  Add
// e.g. ?tags=103,104,201 to reprint just 3 tags.
?>
<html moznomarginboxes mozdisallowselectionprint>
<head>
<title>Car Tags</title>
<!--

This page prints correctly from Firefox.

Be sure to:
    - Select landscape orientation in the print dialog
    - UNCHECK "Ignore scaling and shrink to page width"

-->
<style>
@page  
{ 
  size: 8.5in 10.75in;
  margin: 0mm 0mm 0mm 0mm;  
} 

table {
  border-collapse: collapse;
}

td {
  height: 8.25in;
  margin-left: auto;
  margin-right: auto;
  align: center;
  vertical-align: bottom;
  /*
     border-top: 1px solid black; 
     border-bottom: 1px solid black;
  */
}

td.inner {
   width: 2.7in;
   border-left: 1px solid black;
   border-right: 1px solid black;
}

td.outer {
  /* Outer table cells lose 1/8" to paper border (Firefox) */
  width: 2.625in;
}

.barcode {
  width: 100px;
  margin-left: auto;
  margin-right: auto;
}

div.rank-image {
  width: 100%;
 }

.rank-image img {
  height: 2in;
  margin-left: auto;
  margin-right: auto;
  display: block;
 }

td p.directive {
  text-align: center;
  font-size: 0.25in;
  margin-top: 0in;
}

td p.tagno {
  text-align: center;
  font-size: 0.5in;
  margin-top: 0in;
}

.gap {
  height: 2in;
 }
</style>
</head>
<body>
<table>
<?php

function write_tag($tag, $classes) {
  $rank = floor($tag / 100);
?>
      <td class="<?php echo $classes; ?>">
        <p class="directive">Turn in with car</p>
        <div class="barcode">
          <img src="./barcode.php?orientation=vertical&size=100&stretch=2&text=PWD<?php echo $tag; ?>"/>
        </div>
        <div class="gap">&nbsp;</div>
        <div class="rank-image">
          <img src="ranks/rank<?php echo $rank; ?>.jpg"/>
        </div>
        <p class="tagno"><?php echo $tag; ?></p>
      </td>
<?php
}

function ad_hoc($tags) {
  $count = 0;
  foreach ($tags as $tag) {
    $col = $count % 4;
    if ($col == 0) {
      if ($count > 0) echo "</tr>\n";
      echo "<tr>";
    }
    write_tag($tag, ($col == 0 || $col == 3) ? 'outer' : 'inner');
    ++$count;
  }
  echo "</tr>\n";
}

// $ntags is supposed to be a multiple of 4
function print_full_rank($rank, $ntags) {
    for ($row = 0; $row * 4 < $ntags; ++$row) {
      echo "<tr>\n";
      for ($col = 0; $col < 4; ++$col) {
        // Horizontal layout: $rank * 100 + ($row * 4 + $col + 1)
        write_tag($rank * 100 + $col * ($ntags / 4) + $row + 1,
                  ($col == 0 || $col == 3) ? 'outer' : 'inner');
      }
      echo "<tr/>\n";
    }
}

function sequential_tags() {
  for ($rank = 1; $rank < 6; ++$rank) {
    print_full_rank($rank, $rank == 5 ? 16 : 32);
  }
}

if (isset($_GET['tag']) || isset($_GET['tags'])) {
  $tags = explode(',', @$_GET['tag'].@$_GET['tags']);
  ad_hoc($tags);
} else {
  sequential_tags();
}

?>
</table>
</body>
</html>