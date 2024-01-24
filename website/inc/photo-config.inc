<?php
require_once('inc/data.inc');

// Render labels are:
//  width 'x' height  or
//  height 'h'

define('RENDER_ORIGINAL', 'original');
define('RENDER_CROPPED', 'cropped');
// "Working size" images: big enough to edit, but small enough to fit on screen
// ... based on ORIGINAL not CROPPED
define('RENDER_WORKING', 'work');

// These smaller sizes represent thumbnail images for different purposes.
// Rather than re-render to pixel-specific sizes, it's much more sensible to
// agree on one modest size for all of these, and then use css
// max-height/max-width where needed.

// jQuery Mobile limits images in a listview to be no more than 80 pixels,
// but this can be managed with css.
define('RENDER_LISTVIEW', '200x200');  // formerly '80x80'
define('RENDER_THUMBNAIL', '200x200');  // formerly '150x150'
define('RENDER_CHECKIN', '200x200');  // formerly '0x60'
define('RENDER_JUDGING', '200x200');  // formerly '100x100'
define('RENDER_ONDECK', '400x400');  // formerly 200x100
define('RENDER_RACER_RESULTS', '200x200');  // formerly 68h

function photo_directory() {
  // For Windows:
  // %userprofile%\Documents\Lisano Enterprises\GrandPrix Race Manager\Images
  // or %userprofile%\My Documents\Lisano Enterprises\GrandPrix Race Manager\Images
  return read_raceinfo('photo-directory');
}

function car_photo_directory() {
  $dir = read_raceinfo('car-photo-directory');
  if ($dir === false) {
    return read_raceinfo('photo-directory');
  } else {
    return $dir;
  }
}

// A photo repository refers to a directory on the server where we look for
// original photos.  We keep separate PhotoRepository objects for racer and car
// photos, but it's OK for them to refer to the same directory.
class PhotoRepository {
  private $repo_name;
  private $directory;
  private $column_name;
  private $all_renders = array();

  public function __construct($name, $dir, $columnname) {
    $this->repo_name = $name;
    $this->directory = $dir;
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
  public function url_for_racer(&$row, $render_name) {
    return $this->url_for_file($row[$this->column_name()], $row['racerid'], $render_name);
  }

  public function url_for_file($imagefile, $racerid, $render_name) {
    return $this->lookup_or_any($render_name)->url_for_file($imagefile, $racerid);
  }

  // This only gives the list of renders that have been used for this PHP page
  // instance -- custom renders created via lookup() for another page instance
  // are completely unknown.
  public function all_renders() {
    return $this->all_renders;
  }

  public function lookup($label) {
    $render = @$this->all_renders[$label];
    if (!$render) {
      if (strpos($label, 'x') !== false) {
        list($maxw, $maxh) = explode('x', $label, 2);
        if (is_numeric($maxw) && is_numeric($maxh)) {
          $render = $this->make_render($label, 0 + $maxw, 0 + $maxh, RENDER_CROPPED);
        }
      } else if (substr($label, -1) == 'h') {
        $maxh = substr($label, 0, -1);
        if (is_numeric($maxh)) {
          $render = $this->make_render($label, 0, 0 + $maxh, RENDER_CROPPED);
        }
      }
    }
    return $render;
  }

  public function lookup_or_any($label) {
    $v = $this->lookup($label);
    if ($v) {
      return $v;
    }
    // Pick an arbitrary render if we can't find the requested one.
    foreach ($this->all_renders as $k => $v) {
      return $v;
    }
  }

  public function name() {
    return $this->repo_name;
  }

  public function directory() {
    return $this->directory;
  }

  // 'ok', 'readonly', 'trouble'
  public function status() {
    if (!is_dir($this->directory)) {
      return 'trouble';
    } else if (!is_writable($this->directory)) {
      return 'readonly';
    } else {
      return 'ok';
    }
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
    if ($label != RENDER_ORIGINAL) {
      if ($antecedent != RENDER_CROPPED) {
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
    $directory = $this->directory;
    if (substr($directory, -1) != DIRECTORY_SEPARATOR) {
      $directory .= DIRECTORY_SEPARATOR;
    }
    return $directory.$imagename;
  }

  public function target_file_path($imagename) {
    if (!file_exists($this->directory())) {
      // Make the subdirectory world writable -- typically we'll be running as
      // under the web server as _www or something, and we want the photos to be
      // conveniently accessible by the "native" machine user.
      $old_mask = umask(0);
      // TODO - This fails (loudly) if the parent of $this->directory() doesn't exist, either
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
  // renders.  Invoked on 'cropped' render when (re)cropping a photo, and on
  // 'original' render when rotating.  Note that it works correctly on
  // 'original' (i.e., deleting both cropped and other renders) only because of
  // the particular subfolder structure we use.
  public function delete_dependents($image_name) {
    $mydir = $this->directory();
    if (substr($mydir, -1) != DIRECTORY_SEPARATOR) {
      $mydir .= DIRECTORY_SEPARATOR;
    }
    foreach(glob($this->repository->directory().DIRECTORY_SEPARATOR.'*'.DIRECTORY_SEPARATOR.$image_name) as $path) {
      if (substr($path, 0, -strlen($image_name)) != $mydir) {
        @unlink($path);
      }
    }
  }

  public function cache_breaker_value($imagefile) {
    $breaker = '';
    if ($imagefile != '') {
      $basename = basename($imagefile);
      $breaker = @filemtime($this->best_available_file_path($basename));
      // If best_available_file_path can't find any suitable version of the file
      // at all, $breaker will be FALSE from filemtime; use a 0 instead.
      if (!$breaker) {
        $breaker = 0;
      }
    }
    return $breaker;
  }

  // Constructs a file URL for the photo whose base name is $imagefile.
  public function render_url($imagefile) {
    if ($imagefile != '') {
      $basename = basename($imagefile);
      $breaker = $this->cache_breaker_value($basename);
      return 'photo.php/'.
          $this->repository->name().
          '/file/'.
          $this->label().'/'.
          urlencode($basename).'/'.
          $breaker;
    } else {
      return '';
    }
  }

  // render_url takes a base name; url_for_file takes a full path name
  public function url_for_file($imagefile, $racerid) {
    if ($this->repository->recognized_path($imagefile)) {
      return $this->render_url(basename($imagefile));
    } else if (!file_exists($imagefile) &&
               file_exists($this->repository->lookup_or_any(RENDER_ORIGINAL)
                           ->file_path(basename($imagefile)))) {
      // If the field points to a non-existent file, but the image IS present in
      // the repository, then assume that it's a case of having moved the
      // repository, and treat as a recognized path
      return $this->render_url(basename($imagefile));
    } else {
      // A racer URL
      return 'photo.php/'.$this->repository->name().'/racer/'.$racerid.'/unrec-'.time();
    }
  }

  // $row has to have racerid and the repository's column_name, at least
  public function url_for_row(&$row) {
    return $this->url_for_file($row[$this->repository->column_name()], $row['racerid']);
  }

  public function url_elements_for_row(&$row) {
    $filepath = $row[$this->repository->column_name()];
    if ($filepath) {
      $basename = basename($filepath);
      return array('basename' => $basename,
                   'cache-breaker' => $this->cache_breaker_value($basename));
    } else {
      return null;
    }
  }

  public function find_or_make_image_file($image_name) {
    $target_file_path = $this->target_file_path($image_name);
    if (!file_exists($target_file_path)) {
      $source = $this->find_source_render($image_name);
      if (!$source) {
        return null;
      }
      $source_file_path = $source->file_path($image_name);
      if (substr($source_file_path, -4) == '.svg') {
        return $source_file_path;
      }
      if ($this->max_height() == 0 && $this->max_width() == 0) {
        return $source_file_path;
      } else {
        list($source_width, $source_height, $image_type) = @getimagesize($source_file_path);
        if ($source_height == 0) {
          return false;
        }
        $im = open_image($source_file_path, $image_type);
        if ($im === false) {  // Unsupported picture type
          return false;
        }

        $aspect = $source_width / $source_height;

        if ($this->max_height() == 0 ||
            $this->max_width() > 0 && $aspect > $this->max_width() / $this->max_height()) {
          // Scale to convert $source_width to $max_width if either (a) there's
          // no max_height, or (b) scaling to max height would make a result
          // that's too wide.
          //
          // The scale factor is then $max_width/$source_width.
          $width = $this->max_width();
          $height = (int) ($source_height * $this->max_width() / $source_width);
        } else {
          // Conversely, scale to max_height.
          $width = (int) ($source_width * $this->max_height() / $source_height);
          $height = $this->max_height();
        }

        $thumb = imagecreatetruecolor($width, $height);
        
        // Alpha channel handling for formats that have them (png)
        imagealphablending($thumb, false);
        imagesavealpha($thumb, true);

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


// Supported:
// IMAGETYPE_GIF
// IMAGETYPE_JPEG
// IMAGETYPE_PNG

// Not supported:
// IMAGETYPE_WEBP
// IMAGETYPE_JPEG2000
// IMAGETYPE_SWF
// IMAGETYPE_PSD
// IMAGETYPE_BMP
// IMAGETYPE_WBMP
// IMAGETYPE_XBM
// IMAGETYPE_TIFF_II
// IMAGETYPE_TIFF_MM
// IMAGETYPE_IFF
// IMAGETYPE_JB2
// IMAGETYPE_JPC
// IMAGETYPE_JP2
// IMAGETYPE_JPX
// IMAGETYPE_SWC
// IMAGETYPE_ICO
// IMAGETYPE_AVIF


function open_image($filepath, $image_type) {
  switch ($image_type) {
  case IMAGETYPE_GIF:
    return imagecreatefromgif($filepath);
  case IMAGETYPE_JPEG:
    return imagecreatefromjpeg($filepath);
  case IMAGETYPE_PNG:
    return imagecreatefrompng($filepath);
  }

  return false;
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
  return false;
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
    $headshots = new PhotoRepository('head', photo_directory(), 'imagefile');

    $headshots->make_render(RENDER_ORIGINAL, 0, 0, '');
    $headshots->make_render(RENDER_CROPPED, 0, 0, RENDER_ORIGINAL);
    // "Working size" images: big enough to edit, but small enough to fit on screen
    $headshots->make_render(RENDER_WORKING, 600, 600, RENDER_ORIGINAL);
  }

  return $headshots;
}

function car_photo_repository() {
  global $car_photo_repository;

  if (!isset($car_photo_repository)) {
    $car_photo_repository = new PhotoRepository('car', car_photo_directory(), 'carphoto');

    $car_photo_repository->make_render(RENDER_ORIGINAL, 0, 0, '');
    $car_photo_repository->make_render(RENDER_CROPPED, 0, 0, RENDER_ORIGINAL);
    // "Working size" images: big enough to edit, but small enough to fit on screen
    $car_photo_repository->make_render(RENDER_WORKING, 600, 600, RENDER_ORIGINAL);
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
