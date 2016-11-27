<?php

// The facedetection code is based on the OpenCV (http://opencv.org)
// implementation of Viola-Jones, and uses classifier data expressed as PHP data
// structures as follows:
//
// <classifier> ::= [ [ <width>, <height> ], <stage>... ]
// <stage> ::= [ <stage_threshold>, <weak>... ]
// <weak> ::= <node>
// <node> ::= [ <threshold>, <feature>, <left>, <right> ]
//   <threshold> ::= number
//   <left>, <right> ::= <value> (if is_number) OR
//                   ::= <node>
// <feature> ::= [ x, y, width, height, weight, x, y, width, height, weight ] OR
// <feature> ::= [ x, y, width, height, weight, x, y, width, height, weight, x, y, width, height, weight ]
//
// This script converts an xml file of classifier data, as found in the OpenCV
// project, into a serialized PHP data file.
//
// E.g., convert_from_xml('facedetection2/haarcascade_frontalface_alt2.xml', 'facedetection2/alt2.ser');

function convert_xml($infile, $outfile) {
  $xml = simplexml_load_file($infile);
  $classifier = new CascadeClassifier($xml->xpath('cascade')[0]);
  file_put_contents($outfile, serialize($classifier->emit()));
}

function purge_empty(&$pieces) {
  if (count($pieces) > 0 && empty($pieces[0])) {
    array_shift($pieces);
  }
}
function split_text($txt) {
  $pieces = preg_split('/\s+/', $txt);
  purge_empty($pieces);
  return $pieces;
}

function as_int($x) {
  if (!is_int($x)) {
    $xx = $x>>0;
    if ($xx != $x) {
      trigger_error("Can't make integer from ".$x);
    }
    $x = $xx;
  }
  return $x;
}

// Rects have inclusive lower bounds (x,y) that go to zero,
// and exclusive upper bounds (x + width, y + height) that can reach window size limits
class Rect {
  private $x;
  private $y;
  private $width;
  private $height;
  private $weight;

  // xpath would be cascade/features/_/rects/_ to a single node
  function __construct($underscore) {
    $pieces = split_text((string) $underscore);
    $this->x = as_int(array_shift($pieces));
    $this->y = as_int(array_shift($pieces));
    $this->width = as_int(array_shift($pieces));
    $this->height = as_int(array_shift($pieces));
    $this->weight = array_shift($pieces) + 0;
    if (!is_int($this->x) || !is_int($this->y) || !is_int($this->width) || !is_int($this->height)) {
      trigger_error("Non-integer rect coordinate: ".(string) $underscore);
    }
    purge_empty($pieces);
    if (count($pieces) > 0) {
      trigger_error("Rect failure: ".(string) $underscore);
    }
  }

  function emit_to(&$value) {
    $value[] = $this->x;
    $value[] = $this->y;
    $value[] = $this->width;
    $value[] = $this->height;
    $value[] = $this->weight;
  }
}

class Feature {
  private $rects;

  // xpath would be cascade/features/_
  function __construct($underscore) {
    if (count($underscore->xpath('rects')) != 1) {
      trigger_error("Ill-formed feature: ".$underscore->__toString());
    }
    if (count($underscore->xpath('rects/_')) > 3) {
      trigger_error("Ill-formed feature: ".$underscore->__toString());
    }
    $this->rects = array();
    foreach ($underscore->xpath('rects/_') as $rect) {
      $this->rects[] = new Rect($rect);
    }
  }

  function emit() {
    $value = array();
    foreach ($this->rects as $rect) {
      $rect->emit_to($value);
    }
    return $value;
  }
}

class InternalNode {
  private $left;   // <= 0 for a leaf value,
  private $right;  // > 0 for another InternalNode
  private $feature_index;
  private $threshold;

  function __construct(&$pieces) {
    $this->left = as_int(array_shift($pieces));
    $this->right = as_int(array_shift($pieces));
    $this->feature_index = as_int(array_shift($pieces));
    $this->threshold = array_shift($pieces) + 0;
  }

  function emit(&$internals, &$leaves, &$features) {
    $value = array();
    $value[] = $this->threshold;
    $value[] = $features[$this->feature_index]->emit();
    $value[] = $this->emit_sub($this->left, $internals, $leaves, $features);
    $value[] = $this->emit_sub($this->right, $internals, $leaves, $features);
    return $value;
  }
  function emit_sub($selector, &$internals, &$leaves, &$features) {
    if ($selector <= 0) {
      return $leaves[-$selector];
    } else {
      return $internals[$selector]->emit($internals, $leaves, $features);
    }
  }
};

class WeakClassifier {
  private $internals;
  private $leaves;

  // xpath is cascade/stages/_/weakClassifiers/_
  function __construct($underscore) {
    if (count($underscore->xpath('leafValues')) != 1) {
      trigger_error("Ill-formed Weak Classifer: no leaf values: ".$underscore->__toString());
    }
    $this->internals = array();
    $this->leaves = array();
    $pieces = split_text((string) $underscore->xpath('leafValues')[0]);
    while (count($pieces) > 0) {
      $this->leaves[] = array_shift($pieces) + 0;
    }
    if (count($underscore->xpath('internalNodes')) != 1) {
      tigger_error("Ill-formed Weak Classifier: no internal nodes: ".$underscore->__toString());
    }
    $pieces = split_text((string) $underscore->xpath('internalNodes')[0]);
    while (count($pieces) > 0) {
      $this->internals[] = new InternalNode($pieces);
    }
  }

  function emit(&$features) {
    return $this->internals[0]->emit($this->internals, $this->leaves, $features);
  }
}

class Stage {
  private $weaks;  // weak classifiers
  private $stage_threshold;

  // xpath is cascade/stages/_
  function __construct($underscore) {
    // stageThreshold
    $this->stage_threshold = 0 + (string) ($underscore->xpath('stageThreshold')[0]);
    
    $this->weaks = array();
    foreach ($underscore->xpath('weakClassifiers/_') as $underscore) {
      $this->weaks[] = new WeakClassifier($underscore);
    }

    $weak = $underscore->xpath('maxWeakCount');
    if (count($weak) == 0) {
    } else if (count($weak) == 1) {
      echo "maxWeakCount: ".((string) ($underscore->xpath('maxWeakCount')[0]))."\n";
      $maxWeakCount = ((string) ($underscore->xpath('maxWeakCount')[0]))>>0;
      if (count($this->weaks) != $maxWeakCount) {
        trigger_error("Wrong number of weak classifiers (".$maxWeakCounts." vs ".count($this->weaks).")");
      }
    } else {
      trigger_error("Too many maxWeakCount: ".$underscore->__toString());
    }
  }

  function emit(&$features) {
    $value = array();
    $value[] = $this->stage_threshold;
    foreach ($this->weaks as $weak) {
      $value[] = $weak->emit($features);
    }
    return $value;
  }
}

class CascadeClassifier {
  public $window_width;
  public $window_height;
  private $features;
  private $stages;

  // xpath is just 'cascade'
  function __construct($cascade) {
    $this->window_width = ((string) ($cascade->xpath('width')[0]))>>0;
    $this->window_height = ((string) ($cascade->xpath('height')[0]))>>0;

    $this->features = array();
    foreach ($cascade->xpath('features/_') as $underscore) {
      $this->features[] = new Feature($underscore);
    }

    $this->stages = array();
    foreach($cascade->xpath("stages/_") as $underscore) {
      $this->stages[] = new Stage($underscore);
    }
  }

  function emit() {
    $value = array();
    $value[] = array($this->window_width, $this->window_height);
    foreach ($this->stages as $stage) {
      $value[] = $stage->emit($this->features);
    }
    return $value;
  }
}
?>