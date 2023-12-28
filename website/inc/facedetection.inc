<?php
// Face detection algorithm cribbed from http://opencv.org's cascadedetect.
// Detector data in companion facedetection-data directory derived from opencv's
// haarcascade_frontalface_alt2.xml.

class FaceDetector {
  // As fast as Haar cascades are, this implementation in PHP can run for a
  // minute or more if not time-limited.
  private $max_time_allowed_sec = 6;

  private $window_width;
  private $window_height;
  private $stages;

  // Width and height of the image being analyzed
  private $width;
  private $height;

  // Grayscale representation of the image being analyzed.
  //
  // $gray[ $horiz + $this->width * $vert ] gives the gray value for the pixel
  // at ($horiz, $vert), 0 <= gray < 256,
  private $gray;
  
  // $integral is an array corresponding to a (width + 1)x(height + 1) matrix.
  // $integral[ $horiz + ($this->width + 1) * $vert ] gives the sum of all the
  // grayscale values above and to the left of the pixel at ($horiz, $vert)
  // $integral2 gives the sum of the squares of the same grayscale values.
  private $integral;
  private $integral2;

  private $scale_factor;

  function __construct($data_file_name = 'alt2.dat') {
    $data_file = dirname(__FILE__).DIRECTORY_SEPARATOR.'facedetection-data'.DIRECTORY_SEPARATOR.$data_file_name;
    $classifier_data = file_get_contents($data_file);
    if ($classifier_data) {
      $classifier_data = unserialize($classifier_data);
      $dims = array_shift($classifier_data);
      $this->window_width = array_shift($dims);
      $this->window_height = array_shift($dims);
      $this->stages = $classifier_data;
      $this->scale_factor = 1.1;
    } else {
      trigger_error("No such data file: ".$data_file, E_USER_ERROR);
    }
  }

  function detect_face(&$img) {
    $this->width = imagesx($img);
    $this->height = imagesy($img);
    // Resample down to 320x240
    if ($this->width > 320 || $this->height > 240) {
      $aspect = $this->width / $this->height;
      if ($aspect > 4/3) {
        $scale = $this->width / 320;
        $width = 320;
        $height = ($this->height * 320 / $this->width)>>0;
      } else {
        $scale = $this->height / 240;
        $height = 240;
        $width = ($this->width * 240 / $this->height);
      }
      $width = $width>>0;
      $height = $height>>0;
      $smaller = imagecreatetruecolor($width, $height);
      imagecopyresampled($smaller, $img, 0,0,0,0, $width, $height, $this->width, $this->height);
      $this->width = $width;
      $this->height = $height;
      $v = $this->detect_face1($smaller);
      imagedestroy($smaller);
      return array('x' => ($v['x'] * $scale) >> 0,
                   'y' => ($v['y'] * $scale) >> 0,
                   'width' => ($v['width'] * $scale) >> 0,
                   'height' => ($v['height'] * $scale) >> 0);
    } else {
      return $this->detect_face1($img);
    }
  }

  function detect_face1(&$img) {
    $start_time = time();
    if ($this->width != imagesx($img) || $this->height != imagesy($img)) {
      trigger_error("Image is ".imagesx($img)."x".imagesy($img).
                    ", but it's supposed to be ".$this->width."x".$this->height);
    }
    $this->ToGrayscale($img);
    $this->EqualizeHistogram();
    $this->ComputeIntegrals();

    $stride = $this->width + 1;  // number of entries per row in $this->integral and $this->integral2

    $initial_scale = min($this->width/$this->window_width, $this->height/$this->window_height);
    
    for ($factor = $initial_scale; $factor >= 1.0; $factor /= $this->scale_factor) {
      if (time() - $start_time > $this->max_time_allowed_sec) {
        error_log('Time limit forces abandoning of scale factor loop');
        break;
      }
      $window_width = (0.5 + $factor * $this->window_width)>>0;
      $window_height = (0.5 + $factor * $this->window_height)>>0;
      $window_area = $window_width * $window_height;
      if ($window_width > $this->width || $window_height > $this->height) {
        continue;
      }
      $step = max($factor, 2) >> 0;

      $horiz_limit = $this->width - $window_width;
      $vert_limit = $this->height - $window_height;
      for ($horiz = 0; $horiz < $horiz_limit; $horiz += $step) {
        if (time() - $start_time > $this->max_time_allowed_sec) {
          error_log('Time limit forces abandoning of horizontal loop');
          break;
        }
        for ($vert = 0; $vert < $vert_limit; $vert += $step) {
          if (time() - $start_time > $this->max_time_allowed_sec) {
            error_log('Time limit forces abandoning of vertical loop');
            break;
          }
          $mean = (
            $this->integral[ $horiz                 + $stride * $vert                    ] +
            $this->integral[ $horiz + $window_width + $stride * ($vert + $window_height) ] -
            $this->integral[ $horiz                 + $stride * ($vert + $window_height) ] -
            $this->integral[ $horiz + $window_width + $stride * $vert                    ] ) / $window_area;
          $norm = sqrt( (
            $this->integral2[ $horiz                 + $stride * $vert                    ] +
            $this->integral2[ $horiz + $window_width + $stride * ($vert + $window_height) ] -
            $this->integral2[ $horiz                 + $stride * ($vert + $window_height) ] -
            $this->integral2[ $horiz + $window_width + $stride * $vert                    ] ) / $window_area
                        - $mean * $mean);
          if ($norm < 1) { $norm = 1; }

          $found = true;
          foreach ($this->stages as &$stage) {
            $stage_threshold = $stage[0];
            $stage_total = 0;
            $count_stage = count($stage);
            for ($weak_i = 1; $weak_i < $count_stage; ++$weak_i) {
              $node = $stage[$weak_i];
              while (!is_numeric($node)) {
                $node_threshold = $node[0];
                $feature = &$node[1];
                $feature_total = 0;
                $count_feature = count($feature);
                for ($f = 0; $f < $count_feature; $f += 5) {
                  if (time() - $start_time > $this->max_time_allowed_sec) {
                    error_log('Time limit forces abandoning of feature count loop');
                    break;
                  }
                  $x = (($factor * $feature[$f + 0])>>0) + $horiz;
                  $y = (($factor * $feature[$f + 1])>>0) + $vert;
                  $w = ($factor * $feature[$f + 2])>>0;
                  $h = ($factor * $feature[$f + 3])>>0;
                  $wt = $feature[$f + 4];

                  $score = (
                    $this->integral[ $x      + $stride * $y        ] +
                    $this->integral[ $x + $w + $stride * ($y + $h) ] -
                    $this->integral[ $x      + $stride * ($y + $h) ] -
                    $this->integral[ $x + $w + $stride * $y        ] );
                  $feature_total += $score * $wt;
                }
                if ($feature_total / $window_area < $node_threshold * $norm) {
                  $node = &$node[2];
                } else {
                  $node = &$node[3];
                }
              }
              $stage_total += $node;
            }
            if ($stage_total < $stage_threshold) {
              $found = false;
              break;
            }
          }
          if ($found) {
            return array('x' => $horiz, 'y' => $vert, 'width' => $window_width, 'height' => $window_height);
          }
        }  // vert
      }  // horiz
    }  // factor
    return false;
  }
  
  // $src is the source image of dimensions $this->width by $this->height.
  // Allocates and populates $this->gray
  function ToGrayscale(&$src) {
    $this->gray = new SplFixedArray( $this->width * $this->height );
    for ($vert = 0; $vert < $this->height; ++$vert) {
      for ($horiz = 0; $horiz < $this->width; ++$horiz) {
        $rgb = ImageColorAt($src, $horiz, $vert);
        // These grayscale coefficients taken from OpenCV's RGB2Gray in
        // modules/imgproc/src/color.cpp
        $gray_value =
            0.299 * (($rgb >> 16) & 0xFF) +
            0.587 * (($rgb >> 8) & 0xFF) +
            0.114 * ($rgb & 0xFF);
        if ($gray_value < 0 || $gray_value > 255) {
          trigger_error("Gray out of bounds, color ".dechex($rgb)." -> gray ".$gray_value);
        }
        $this->gray[$horiz + $vert * $this->width] = $gray_value>>0;
      }
    }
  }

  function EqualizeHistogram() {
    $histo = new SplFixedArray(256);
    for ($i = 0; $i < 256; ++$i) { $histo[$i] = 0; }
    $size = count($this->gray);

    // Capture histogram
    for ($i = 0; $i < $size; ++$i) {
      $histo[$this->gray[$i]] += 1;
    }

    // Convert to normalized cumulative histogram
    $running_count = 0;
    for ($i = 0; $i < 256; ++$i) {
      $running_count += $histo[$i];
      $histo[$i] = ((255 * $running_count) / $size) >> 0;
    }

    if ($running_count != $size) {
      trigger_error("Screwed up: ".$running_count." out of ".$size);
    }
  
    // Equalize by applying normalized cumulative histogram mapping
    for ($i = 0; $i < $size; ++$i) {
      $this->gray[$i] = $histo[$this->gray[$i]];
    }
  }

  // For testing and debugging, convert $gray back into an image.
  function ToImage() {
    $image = imagecreatetruecolor($this->width, $this->height);
    $colors = new SplFixedArray(256);
    for ($i = 0; $i < 256; ++$i) {
      $colors[$i] = imagecolorallocate($image, $i, $i, $i);
    }
    for ($horiz = 0; $horiz < $this->width; ++$horiz) {
      for ($vert = 0; $vert < $this->height; ++$vert) {
        imagesetpixel($image, $horiz, $vert, $colors[$this->gray[$horiz + $vert * $this->width]]);
      }
    }
    return $image;
  }

  function ComputeIntegrals() {
    $this->integral = new SplFixedArray( ($this->width + 1) * ($this->height + 1) );
    $this->integral2 = new SplFixedArray( ($this->width + 1) * ($this->height + 1) );
    $stride = $this->width + 1;
    // First row is zeros, because there's nothing above to be summed
    for ($i = 0; $i < $stride; ++$i) {
      $this->integral[$i] = 0;
      $this->integral2[$i] = 0;
    }
    for ($vert = 0; $vert < $this->height; ++$vert) {
      $rowtotal = 0;
      $rowtotal2 = 0;
      for ($horiz = 0; $horiz < $this->width; ++$horiz) {
        $this->integral[$horiz + $stride * ($vert + 1)] = $this->integral[$horiz + $stride * $vert] + $rowtotal;
        $this->integral2[$horiz + $stride * ($vert + 1)] = $this->integral2[$horiz + $stride * $vert] + $rowtotal2;
        $g = $this->gray[$horiz + $this->width * $vert];
        $rowtotal += $g;
        $rowtotal2 += $g * $g;
      }
      $this->integral[$this->width + $stride * ($vert + 1)] = $this->integral[$this->width + $stride * $vert] + $rowtotal;
      $this->integral2[$this->width + $stride * ($vert + 1)] = $this->integral2[$this->width + $stride * $vert] + $rowtotal2;
    }
  }
}

?>
