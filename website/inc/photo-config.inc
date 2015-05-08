<?php
require_once('inc/data.inc');

function photo_directory() {
  // PHP_OS == 'Darwin'
  return read_raceinfo('photo-directory', 'C:\\racer_photos');
}

function car_photo_directory() {
  $dir = read_raceinfo('car-photo-directory');
  if ($dir === false) {
    return read_raceinfo('photo-directory', 'C:\\car_photos');
  } else {
    return $dir;
  }
}

function photo_size() {
  return read_raceinfo('photo-size', '94x125');
}

function car_photo_size() {
  $size = read_raceinfo('car-photo-size');
  if ($size === false) {
    return photo_size();
  } else {
    return $size;
  }
}

// A photo repository refers to a directory on the server where we look for
// original photos.  We keep separate PhotoRepository objects for racer and car
// photos, but it's OK for them to refer to the same directory.
class PhotoRepository {
  private $repo_name;
  private $directory;
  private $display_width;
  private $display_height;
  private $column_name;
  private $all_renders = array();

  // $widthxheight tells the "display" dimensions for pictures
  public function __construct($name, $dir, $widthxheight, $columnname) {
    $this->repo_name = $name;
    $this->directory = $dir;
    $photosize = explode('x', $widthxheight);
    $this->display_width = 0 + $photosize[0];
    $this->display_height = 0 + $photosize[1];
    $this->column_name = $columnname;
  }

  // This assumes that any file found within any subdirectory of the repository
  // folder is being managed by us.
  public function recognized_path($imagefile) {
    $imagepath = realpath(pathinfo($imagefile, PATHINFO_DIRNAME));
    while ($imagepath !== false && strlen($imagepath) > 1) {
      if ($imagepath == realpath($this->directory())) {
        return true;
      }
      $next_imagepath = dirname($imagepath);
      if ($next_imagepath == $imagepath) {
        return false;
      } else {
        $imagepath = $next_imagepath;
      }
    }
    return false;
  }

  // $row has to have racerid and the repository's column_name, at least
  public function url_for_racer(&$row, $render = 'display') {
    $imagefile = $row[$this->column_name()];
    if ($this->recognized_path($imagefile)) {
      return $this->lookup($render)->render_url(basename($imagefile));
    } else if (!file_exists($imagefile) &&
               file_exists($this->lookup('original')->file_path(basename($imagefile)))) {
      // If the field points to a non-existent file, but the image IS present in
      // the repository, then assume that it's a case of having moved the
      // repository, and treat as a recognized path
      return $this->lookup($render)->render_url(basename($imagefile));
    } else {
      // A racer URL
      return 'photo.php/'.$this->name().'/racer/'.$row['racerid'].'/'.time();
    }
  }

  public function all_renders() {
    return $this->all_renders;
  }

  public function lookup($label) {
    return $this->all_renders[$label];
  }

  public function name() {
    return $this->repo_name;
  }

  public function directory() {
    return $this->directory;
  }

  public function display_width() {
    return $this->display_width;
  }

  public function display_height() {
    return $this->display_height;
  }

  public function column_name() {
    return $this->column_name;
  }

  public function make_render($label, $max_width, $max_height, $antecedent) {
    return $this->all_renders[$label] = 
        new PhotoRender($this, $label, $max_width, $max_height, $antecedent);
  }
}

// A photo "render" here is a subdirectory of the photo repository, into which
// are written cropped and/or scaled versions of the original photos.
class PhotoRender {

  public function __construct($repository, $label, $max_width, $max_height, $antecedent) {
    $this->repository = $repository;
    $this->label = $label;
    $this->directory = $repository->directory();
    if ($label != 'original') {
      if ($antecedent != 'cropped') {
        $this->directory .= DIRECTORY_SEPARATOR.'0-'.$label;
      } else {
        $this->directory .= DIRECTORY_SEPARATOR.$max_width.'x'.$max_height;
      }
    }

    $this->max_width = $max_width;
    $this->max_height = $max_height;
    $this->antecedent = $antecedent;
  }

  public function repository() { return $this->repository; }
  public function label() { return $this->label; }
  public function directory() { return $this->directory; }
  public function max_width() { return $this->max_width; }
  public function max_height() { return $this->max_height; }
  public function antecedent() { return $this->antecedent; }

  public function file_path($imagename) {
    return $this->directory.DIRECTORY_SEPARATOR.$imagename;
  }

  public function target_file_path($imagename) {
    if (!file_exists($this->directory())) {
      // Make the subdirectory world writable -- typically we'll be running as
      // under the web server as _www or something, and we want the photos to be
      // conveniently accessible by the "native" machine user.
      $old_mask = umask(0);
      mkdir($this->directory());
      umask($old_mask);
    }
    return $this->file_path($imagename);
  }

  public function best_available_file_path($imagename) {
    $path = $this->file_path($imagename);
    if (file_exists($path)) {
      return $path;
    } else {
      $source = $this->find_source_render($imagename);
      if ($source) {
        return $source->file_path($imagename);
      } else {
        return false;
      }
    }
  }

  // Returns the render from which this render derives its pictures: either the
  // original form or cropped.
  public function find_source_render($image_name) {
    if ($this->antecedent) {
      $source = $this->repository->lookup($this->antecedent);
      if (file_exists($source->file_path($image_name))) {
        return $source;
      } else {
        return $source->find_source_render($image_name);
      }
    } else {
      return false;
    }
  }

  // For a particular image, remove all the derived versions in dependent
  // renders.
  public function delete_dependents($image_name) {
    foreach ($this->repository->all_renders() as $render) {
      if ($render->antecedent() == $this->label()) {
        @unlink($render->file_path($image_name));
        $render->delete_dependents($image_name);
      }
    }
  }

  public function wipe_directory() {
    if (is_dir($this->directory())) {
      foreach (glob($this->directory().DIRECTORY_SEPARATOR.'*.{jpg,jpeg,JPG,JPEG,gif,GIF,png,PNG}', GLOB_BRACE)
               as $file) {
        @unlink($file);
      }
    }
  }

  // Constructs a file URL for the photo whose base name is $imagefile.
  public function render_url($imagefile) {
    if ($imagefile != '') {
      $imagefile = basename($imagefile);
      $targetpath = $this->file_path($imagefile);
      $breaker = @filemtime($targetpath);
      if (!$breaker) {
        // If the file doesn't exist, use the current (request) time as a cache
        // breaker, but negate it to make sure there's no collision with the
        // filemtime when the file does get created.
        $breaker = -time();
      }
      return 'photo.php/'.$this->repository->name().'/file/'.$this->label().'/'.urlencode($imagefile).'/'.$breaker;
    } else {
      return '';
    }
  }

  public function find_or_make_image_file($image_name) {
    $target_file_path = $this->target_file_path($image_name);
    if (!file_exists($target_file_path)) {
      $source = $this->find_source_render($image_name);
      $source_file_path = $source->file_path($image_name);

      if ($this->max_height() == 0) {
        return $source_file_path;
      } else {
        list($source_width, $source_height, $image_type) = getimagesize($source_file_path);
        if ($source_height == 0) {
          return FALSE;
        }

        $im = open_image($source_file_path, $image_type);

        $aspect = $source_width / $source_height;

        if ($aspect > $this->max_width() / $this->max_height()) {
          // At any height, source image is too wide, so pick scaling to
          // convert $source_width to $max_width.  The scale factor is then
          // $max_width/$source_width.
          $width = $this->max_width();
          $height = (int) ($source_height * $this->max_width() / $source_width);
        } else {
          // Conversely, scale to max_height.
          $width = (int) ($source_width * $this->max_height() / $source_height);
          $height = $this->max_height();
        }

        $thumb = imagecreatetruecolor($width, $height);
        imagecopyresampled($thumb, $im, 0,0,0,0, $width, $height, $source_width, $source_height);

        write_image($thumb, $target_file_path, $image_type);

        imagedestroy($im);
        imagedestroy($thumb);
      }
    }
    return $target_file_path;
  }

  private $repository;
  private $label;
  private $directory;
  private $max_height;
  private $max_width;
  private $antecedent;
}

function open_image($filepath, $image_type) {
  switch ($image_type) {
  case IMAGETYPE_GIF:
    return imagecreatefromgif($filepath);
  case IMAGETYPE_JPEG:
    return imagecreatefromjpeg($filepath);
  case IMAGETYPE_PNG:
    return imagecreatefrompng($filepath);
  }

  return FALSE;
}

function write_image($image, $filepath, $image_type) {
  switch ($image_type) {
  case IMAGETYPE_GIF:
    return imagegif($image, $filepath);
  case IMAGETYPE_JPEG:
    return imagejpeg($image, $filepath, 90);
  case IMAGETYPE_PNG:
    return imagepng($image, $filepath, 0);
  }
  return FALSE;
}

function pseudo_mime_content_type($filename) {
  $mime_types = array(
            'txt' => 'text/plain',
            'htm' => 'text/html',
            'html' => 'text/html',
            'php' => 'text/html',
            'css' => 'text/css',
            'js' => 'application/javascript',
            'json' => 'application/json',
            'xml' => 'application/xml',
            'swf' => 'application/x-shockwave-flash',
            'flv' => 'video/x-flv',

            // images
            'png' => 'image/png',
            'jpe' => 'image/jpeg',
            'jpeg' => 'image/jpeg',
            'jpg' => 'image/jpeg',
            'gif' => 'image/gif',
            'bmp' => 'image/bmp',
            'ico' => 'image/vnd.microsoft.icon',
            'tiff' => 'image/tiff',
            'tif' => 'image/tiff',
            'svg' => 'image/svg+xml',
            'svgz' => 'image/svg+xml',

            // archives
            'zip' => 'application/zip',
            'rar' => 'application/x-rar-compressed',
            'exe' => 'application/x-msdownload',
            'msi' => 'application/x-msdownload',
            'cab' => 'application/vnd.ms-cab-compressed',

            // audio/video
            'mp3' => 'audio/mpeg',
            'qt' => 'video/quicktime',
            'mov' => 'video/quicktime',

            // adobe
            'pdf' => 'application/pdf',
            'psd' => 'image/vnd.adobe.photoshop',
            'ai' => 'application/postscript',
            'eps' => 'application/postscript',
            'ps' => 'application/postscript',

            // ms office
            'doc' => 'application/msword',
            'rtf' => 'application/rtf',
            'xls' => 'application/vnd.ms-excel',
            'ppt' => 'application/vnd.ms-powerpoint',

            // open office
            'odt' => 'application/vnd.oasis.opendocument.text',
            'ods' => 'application/vnd.oasis.opendocument.spreadsheet',
        );

  $filename_exploded = explode('.', $filename);
  $ext = strtolower(array_pop($filename_exploded));
  if (array_key_exists($ext, $mime_types)) {
	return $mime_types[$ext];
  }
  elseif (function_exists('finfo_open')) {
	$finfo = finfo_open(FILEINFO_MIME);
	$mimetype = finfo_file($finfo, $filename);
	finfo_close($finfo);
	return $mimetype;
  }
  else {
	return 'application/octet-stream';
  }
}

function headshots() {
  global $headshots;

  if (!isset($headshots)) {
    $headshots = new PhotoRepository('head', photo_directory(), photo_size(), 'imagefile');

    $headshots->make_render('original', 0, 0, '');
    $headshots->make_render('cropped', 0, 0, 'original');
    // "Working size" images: big enough to edit, but small enough to fit on screen
    $headshots->make_render('work', 600, 600, 'original');
    $headshots->make_render('display', $headshots->display_width(), $headshots->display_height(), 'cropped');
    // "Thumbnail" images are what get displayed on the photo-thumbs page
    $headshots->make_render('thumb', 150, 150, 'cropped');
    // "Tiny": no more than 80 pixels, because that's what jQuery Mobile
    // limits to in a listview.
    $headshots->make_render('tiny', 80, 80, 'cropped');
    $headshots->make_render('checkin', 60, 60, 'cropped');
  }

  return $headshots;
}

// TODO: car photo repository should probably have its own display size,
// different from photo_size used for head shots.
function car_photo_repository() {
  global $car_photo_repository;

  if (!isset($car_photo_repository)) {
    $car_photo_repository = new PhotoRepository('car', car_photo_directory(), car_photo_size(), 'carphoto');

    $car_photo_repository->make_render('original', 0, 0, '');
    $car_photo_repository->make_render('cropped', 0, 0, 'original');
    // "Working size" images: big enough to edit, but small enough to fit on screen
    $car_photo_repository->make_render('work', 600, 600, 'original');
    $car_photo_repository->make_render('display', 
                                       $car_photo_repository->display_width(),
                                       $car_photo_repository->display_height(),
                                       'cropped');
    // "Thumbnail" images are what get displayed on the photo-thumbs page
    $car_photo_repository->make_render('thumb', 150, 150, 'cropped');
    // "Tiny": no more than 80 pixels, because that's what jQuery Mobile
    // limits to in a listview.
    $car_photo_repository->make_render('tiny', 80, 80, 'cropped');
  }

  return $car_photo_repository;
}

function photo_repository($name) {
  if ($name == 'head') {
    return headshots();
  } else if ($name == 'car') {
    return car_photo_repository();
  // TODO } else {
  }
}

?>
