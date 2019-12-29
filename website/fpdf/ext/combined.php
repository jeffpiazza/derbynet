<?php

// We build a "chain" of FPDF extensions, as suggested in the FPDF FAQ.

require_once('fpdf/fpdf.php');

require_once('fpdf/ext/transform.php');  // PDF_Transform
require_once('fpdf/ext/alphapdf.php');  // PDF_Alpha
require_once('fpdf/ext/rounded_rect.php');  // PDF_RoundedRect
require_once('fpdf/ext/rpdf.php');  // PDF_TextDirection
require_once('fpdf/ext/circulartext.php');  // PDF_CircularText
require_once('fpdf/ext/gradients.php');  // PDF_Gradients
require_once('fpdf/ext/mem_image.php');  // PDF_MemImage
require_once('fpdf/ext/barcode.php');  // PDF_Barcode
require_once('fpdf/ext/sequencer.php');  // PDF_Sequencer
require_once('fpdf/ext/rotation_and_centering.php');  // PDF_RotationAndCentering
require_once('fpdf/ext/ellipse.php');  // PDF_Ellipse

// We introduce this class to terminate the inheritance chain, so consumers can
// be isolated from the details.
class PDF_Combined extends PDF_Ellipse {
}
