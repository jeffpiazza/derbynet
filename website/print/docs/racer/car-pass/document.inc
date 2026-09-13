<?php
// Save as: /var/www/html/derbynet/print/docs/racer/car-pass/document.inc

require_once('inc/path-info.inc');
require_once('inc/schema_version.inc');
require_once('inc/photo-config.inc');

class CarPassDocument extends PrintableRacerDocument {
  function name() { 
    return "Car Pass"; 
  }

  function get_available_options() {
    // Get design categories for selection
    $categories = array();
    try {
      global $db;
      $stmt = $db->query('SELECT a.awardid, a.awardname 
                         FROM Awards a
                         INNER JOIN AwardTypes at ON a.awardtypeid = at.awardtypeid
                         WHERE at.awardtype IN ("Design General", "Design Trophy")
                         ORDER BY a.awardname');
                         
      $category_values = array(
        array('value' => '0', 'desc' => 'All Categories')
      );
      
      foreach ($stmt as $row) {
        $category_values[] = array(
          'value' => $row['awardid'], 
          'desc' => $row['awardname']
        );
      }
    } catch (Exception $e) {
      // If query fails, just provide the "All Categories" option
      $category_values = array(
        array('value' => '0', 'desc' => 'All Categories')
      );
    }
    
    return array(
      'layout' => array(
        'type' => 'radio',
        'desc' => 'Size',
        'values' => array(
          array('value' => '4-up', 'desc' => '8-1/2 x 11, 4-up'),
          array('value' => '4x6', 'desc' => '4x6')
        )
      ),
      'with_logo' => array(
        'type' => 'bool',
        'desc' => 'Include logo image',
        'default' => true
      ),
      'show_date' => array(
        'type' => 'bool',
        'desc' => 'Show date on car pass',
        'default' => true
      ),
      'date_text' => array(
        'type' => 'string',
        'desc' => 'Date to display',
        'default' => date('F j, Y')
      ),
      'category_filter' => array(
        'type' => 'radio',
        'desc' => 'Filter by Design Category',
        'values' => $category_values
      ),
      'show_categories' => array(
        'type' => 'bool',
        'desc' => 'Show design categories on pass',
        'default' => true
      ),
      'bg_color' => array(
        'type' => 'radio',
        'desc' => 'Background color',
        'values' => array(
          array('value' => 'blue', 'desc' => 'Blue'),
          array('value' => 'green', 'desc' => 'Green'),
          array('value' => 'grey', 'desc' => 'Grey')
        )
      )
    );
  }

  protected $y_coords;

  function StartDocument() {
    $this->set_parameters($this->get_option('layout'));
    if ($this->get_option('layout') == '4x6') {
      $this->initialize_pdf('P', 'in', array(4, 6));
      $this->initialize_layout(3.825, 6);
    } else {
      $this->initialize_pdf('P', 'in', 'Letter');
      $this->initialize_layout(3.825, 5.000);
    }
  }

  function set_parameters($layout) {
    if ($layout == '4-up') {
      $this->y_coords = array(
        'car_number' => 0.35,
        'name' => 0.65,
        'car_photo' => 1.0,
        'categories' => 3.30,  // Moved down from car photo
        'date' => 4.70,        // Moved lower
        'logo' => 4.75,        // Moved even lower
      );
    } else if ($layout == '4x6') {
      $this->y_coords = array(
        'car_number' => 0.35,
        'name' => 0.65,
        'car_photo' => 1.0,
        'categories' => 4.35,  // Moved down from car photo
        'date' => 5.70,        // Moved lower
        'logo' => 5.80,        // Moved even lower
      );
    }
  }

  // Override DrawOne to filter by category if needed
  function DrawOne(&$racer) {
    $category_filter = $this->get_option('category_filter');
    
    // Skip racer if they're not in the selected category
    if ($category_filter != '0') {
      $racer_categories = $this->get_racer_categories($racer['racerid']);
      if (!in_array($category_filter, $racer_categories)) {
        return; // Skip this racer
      }
    }
    
    // Call parent DrawOne to continue with the normal drawing process
    parent::DrawOne($racer);
  }

  function DrawOneAt($x, $y, &$racer) {
    // Set background color based on option - with default if not set
    $bgColor = $this->get_option('bg_color');
    if (!$bgColor) {
      $bgColor = 'grey'; // Default color if none selected
    }
    
    if ($bgColor == 'blue') {
      $this->pdf->SetFillColor(220, 235, 255);
    } else if ($bgColor == 'green') {
      $this->pdf->SetFillColor(220, 255, 220);
    } else {
      $this->pdf->SetFillColor(240, 240, 240);
    }
    
    // Draw background
    if ($this->get_option('layout') == '4x6') {
      $this->pdf->Rect($x, $y, 3.825, 6, 'F');
    } else {
      $this->pdf->Rect($x, $y, 3.825, 5, 'F');
    }
    
    // Draw border
    $this->pdf->SetDrawColor(0, 0, 0);
    if ($this->get_option('layout') == '4x6') {
      $this->pdf->Rect($x, $y, 3.825, 6, 'D');
    } else {
      $this->pdf->Rect($x, $y, 3.825, 5, 'D');
    }
    
    // Draw car number at top
    $this->pdf->SetFont('Helvetica', 'B', 16);
    $this->pdf->SetTextColor(0, 0, 128);
    $this->pdf->CenteredText($x + 3.825/2, $y + $this->y_coords['car_number'], 'Car #'.$racer['carnumber']);
    
    // Draw racer name below car number
    $this->pdf->SetFont('Times', 'B', 14);
    $this->pdf->SetTextColor(0, 0, 0);
    $this->pdf->CenteredText($x + 3.825/2, $y + $this->y_coords['name'], 
                           $racer['firstname'].' '.$racer['lastname']);

    // Get car photo using several methods in order of preference
    $car_photo = $this->find_car_photo($racer);
    
    // Draw car photo if found - LARGER SIZE
    if ($car_photo && file_exists($car_photo)) {
      try {
        // Calculate dimensions to maintain aspect ratio
        list($width, $height) = getimagesize($car_photo);
        $max_width = 3.5;  // Increased from 2.5
        $max_height = 2.35; // Increased from 2.0
        
        if ($width && $height) {
          if ($width/$height > $max_width/$max_height) {
            // Width limited
            $display_width = $max_width;
            $display_height = $height * ($max_width/$width);
          } else {
            // Height limited
            $display_height = $max_height;
            $display_width = $width * ($max_height/$height);
          }
          
          $offset_x = ($max_width - $display_width) / 2;
          $this->pdf->Image($car_photo, 
                          $x + (3.825 - $display_width) / 2, 
                          $y + $this->y_coords['car_photo'], 
                          $display_width);
        } else {
          // If getimagesize failed, just use a fixed size
          $this->pdf->Image($car_photo, 
                          $x + 0.6625, 
                          $y + $this->y_coords['car_photo'], 
                          2.5);
        }
      } catch (Exception $e) {
        // If image fails to load, just continue without it
      }
    }

    // List design categories the racer is entered in - SIMPLIFIED APPROACH
    if ($this->get_option('show_categories')) {
      $categories = $this->get_racer_category_names($racer['racerid']);
      if (!empty($categories)) {
        // Calculate category position
        $cat_y = $y + $this->y_coords['categories'];
        $cat_x = $x + 0.25;
        $cat_width = 3.325;
        
        // Get category text
        $category_text = implode(", ", $categories);
        
        // Create background for categories
        $this->pdf->SetFillColor(235, 235, 255);
        
        // Measure the text to determine how many lines we need
        $this->pdf->SetFont('Times', 'I', 10);
        $line_height = 0.15;
        $available_width = $cat_width - 0.2 - $this->pdf->GetStringWidth("Entered in: ");
        
        // Calculate how many lines we need for the categories
        $textWidth = $this->pdf->GetStringWidth($category_text);
        $lines_needed = max(1, ceil($textWidth / $available_width));
        
        // Draw the background box
        $box_height = ($lines_needed + 0.5) * $line_height;
        $this->pdf->Rect($cat_x, $cat_y - 0.05, $cat_width, $box_height, 'F');
        
        // Print "Entered in:" in bold
        $this->pdf->SetFont('Times', 'B', 10);
        $this->pdf->SetTextColor(0, 0, 0);
        $this->pdf->SetXY($cat_x + 0.1, $cat_y);
        $this->pdf->Write(0.15, "Entered in: ");
        
        // Calculate position after prefix
        $prefix_width = $this->pdf->GetStringWidth("Entered in: ");
        
        // Print categories in italic, forcing it to wrap properly
        $this->pdf->SetFont('Times', 'I', 10);
        $this->pdf->SetXY($cat_x + 0.1 + $prefix_width, $cat_y);
        $this->pdf->MultiCell($cat_width - 0.2 - $prefix_width, $line_height, $category_text, 0, 'L');
      }
    }

    // Draw date if option selected - moved to bottom right
    if ($this->get_option('show_date')) {
      $this->pdf->SetFont('Times', '', 12);
      $this->pdf->SetTextColor(0, 0, 0);
      $this->pdf->SetXY($x + 1.9125, $y + $this->y_coords['date'] - 0.15);
      $this->pdf->Cell(1.9125, 0.15, $this->get_option('date_text'), 0, 0, 'R');
    }

    // Draw logo if option selected - moved to bottom left corner very low
    if ($this->get_option('with_logo')) {
      $emblem = image_file_path('emblem');
      if ($emblem && file_exists($emblem)) {
        $logo_size = 0.75; // Slightly smaller
        // Positioning the logo in the very bottom left
        $bottom_margin = 0.1; // Distance from bottom edge
        
        // Calculate position based on layout
        if ($this->get_option('layout') == '4x6') {
            $logo_y = $y + 6 - $logo_size - $bottom_margin;
        } else {
            $logo_y = $y + 5 - $logo_size - $bottom_margin;
        }
        
        $this->pdf->Image($emblem, $x + 0.15, $logo_y, $logo_size, $logo_size);
      }
    }
  }

  private function find_car_photo(&$racer) {
    try {
      // Get the racer ID and car number
      $racerId = $racer['racerid'];
      $carNumber = $racer['carnumber'];
      
      // Try to get the current database path
      $db_path = '';
      
      // Method 1: Try to extract path from the database connection
      global $db;
      if (isset($db) && method_exists($db, 'query')) {
        $stmt = $db->query("PRAGMA database_list");
        if ($stmt) {
          $db_info = $stmt->fetch(PDO::FETCH_ASSOC);
          if (isset($db_info['file'])) {
            $db_path = dirname($db_info['file']);
          }
        }
      }
      
      // Method 2: Try to get database info from DerbyNet function
      if (empty($db_path) && function_exists('get_database_info')) {
        $db_info = get_database_info();
        if (isset($db_info['path'])) {
          $db_path = dirname($db_info['path']);
        }
      }
      
      // Method 3: Fall back to scanning the directory
      if (empty($db_path)) {
        // Look in the standard location for the latest database
        $base_dir = '/var/lib/derbynet';
        $latest_year = '';
        $latest_db = '';
        
        if (is_dir($base_dir)) {
          $years = scandir($base_dir);
          foreach ($years as $year) {
            if ($year != '.' && $year != '..' && is_dir("$base_dir/$year")) {
              if ($year > $latest_year) {
                $latest_year = $year;
              }
            }
          }
          
          if (!empty($latest_year)) {
            $year_dir = "$base_dir/$latest_year";
            $dbs = scandir($year_dir);
            $latest_time = 0;
            
            foreach ($dbs as $db) {
              if ($db != '.' && $db != '..' && is_dir("$year_dir/$db")) {
                $db_time = filemtime("$year_dir/$db");
                if ($db_time > $latest_time) {
                  $latest_time = $db_time;
                  $latest_db = $db;
                }
              }
            }
            
            if (!empty($latest_db)) {
              $db_path = "$year_dir/$latest_db";
            }
          }
        }
      }
      
      // Fallback to hardcoded path if all dynamic methods fail
      if (empty($db_path)) {
        $db_path = '/var/lib/derbynet/2025/2025 Derby';
      }
      
      // Look in the cars directory with the correct "Carid" prefix pattern
      $car_dir = $db_path . '/cars/';
      if (is_dir($car_dir)) {
        // Try with racerid
        $patterns = array(
          $car_dir . 'Carid' . sprintf('%03d', $racerId) . '.jpg',  // Carid001.jpg
          $car_dir . 'Carid' . $racerId . '.jpg',                    // Carid1.jpg
          $car_dir . 'Carid' . sprintf('%03d', $racerId) . '_1.jpg', // Carid001_1.jpg - variant
          $car_dir . 'Carid' . $racerId . '_1.jpg',                  // Carid1_1.jpg - variant
        );
        
        foreach ($patterns as $path) {
          if (file_exists($path)) {
            return $path;
          }
        }
        
        // Try with car number
        $car_patterns = array(
          $car_dir . 'Carid' . sprintf('%03d', $carNumber) . '.jpg',
          $car_dir . 'Carid' . $carNumber . '.jpg',
          $car_dir . 'Carid' . sprintf('%03d', $carNumber) . '_1.jpg',
          $car_dir . 'Carid' . $carNumber . '_1.jpg',
        );
        
        foreach ($car_patterns as $path) {
          if (file_exists($path)) {
            return $path;
          }
        }
      }
      
      // Method 2: Try using the built-in DerbyNet photo functions
      if (function_exists('photo_file_path') && schema_version() >= 2 && !empty($racer['carphoto'])) {
        $car_photo = photo_file_path('car', $racer['carphoto']);
        if ($car_photo && file_exists($car_photo)) {
          return $car_photo;
        }
      }
      
      // Method 3: Look in other photo directories as fallback
      $photo_dirs = array(
        '/var/www/html/derbynet/Photos/',
        '/var/www/html/derbynet/photos/',
        '/var/www/html/derbynet/car-photos/',
        '/var/www/html/derbynet/Images/cars/',
        '/var/www/html/derbynet/images/cars/'
      );
      
      foreach ($photo_dirs as $dir) {
        if (is_dir($dir)) {
          // Try with Car prefix and .jpg extension
          $car_path = $dir.'Car'.$carNumber.'.jpg';
          if (file_exists($car_path)) {
            return $car_path;
          }
        }
      }
    } catch (Exception $e) {
      // If any error occurs in photo finding, just return false
    }
    
    return false;
  }
  
  // Get the list of design category IDs for a racer
  private function get_racer_categories($racerid) {
    $categories = array();
    try {
      global $db;
      $stmt = $db->prepare("SELECT awardid FROM DesignEntries WHERE racerid = :racerid");
      $stmt->execute(array(':racerid' => $racerid));
      
      while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $categories[] = $row['awardid'];
      }
    } catch (Exception $e) {
      // If there's an error, return an empty array
    }
    
    return $categories;
  }
  
  // Get the list of design category names for a racer
  private function get_racer_category_names($racerid) {
    $categories = array();
    try {
      global $db;
      $stmt = $db->prepare("
          SELECT a.awardname 
          FROM DesignEntries de
          INNER JOIN Awards a ON de.awardid = a.awardid
          INNER JOIN AwardTypes at ON a.awardtypeid = at.awardtypeid
          WHERE de.racerid = :racerid
          AND at.awardtype IN ('Design General', 'Design Trophy')
          ORDER BY a.awardname
      ");
      $stmt->execute(array(':racerid' => $racerid));
      
      while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $categories[] = $row['awardname'];
      }
    } catch (Exception $e) {
      // If there's an error, return an empty array
    }
    
    return $categories;
  }
}
?>
