<?php

// A DocumentLayout object figures out how to lay out multiple copies of a
// printable document on a single piece of paper.
//
// Call the constructor with the $pdf and the width and height of one printable
// document.  By default, as many copies of the document will be laid out on a
// page, with even spacing.
//
// Adjustments to the layout can be made by calling AdjustHorizontalGap and/or
// AdjustVerticalGap.
//
// Once the DocumentLayout is constructed, call NextTopLeft repeatedly to obtain
// coordinates for the top left corner of successive documents.  NextTopLeft
// takes care of allocating physical pages in the pdf, as needed.
class DocumentLayout {
  // The layout's constructor takes an fpdf object, plus the dimensions of a
  // single document we're rendering on that page.
  function __construct($pdf, $doc_width, $doc_height) {
    $this->pdf = $pdf;
    $this->doc_width = $doc_width;
    $this->doc_height = $doc_height;

    $this->n_per_row = floor($pdf->GetPageWidth() / $this->doc_width);
    $this->alloc_width = $pdf->GetPageWidth() / $this->n_per_row;

    $n_per_column = floor($pdf->GetPageHeight() / $this->doc_height);
    $this->alloc_height = $pdf->GetPageHeight() / $n_per_column;

    $this->n_up = $this->n_per_row * $n_per_column;
    $this->CalculateMargins();
    
    $this->count = 0;
  }

  protected function CalculateMargins() {
    $horizontal_gap = $this->alloc_width - $this->doc_width;
    // printed width accounts for n_per_row documents and n_per_row - 1 gaps.
    $printed_width = $this->n_per_row * $this->alloc_width - $horizontal_gap;
    $this->left_margin = ($this->pdf->GetPageWidth() - $printed_width) / 2;

    $n_per_column = $this->n_up / $this->n_per_row;
    $vertical_gap = $this->alloc_height - $this->doc_height;
    $printed_height = $n_per_column * $this->alloc_height - $vertical_gap;
    $this->top_margin = ($this->pdf->GetPageHeight() - $printed_height) / 2;
  }
  
  protected $pdf;

  // Dimensions of a single document on the page.
  protected $doc_width;
  protected $doc_height;

  // Each rendered document on the page is placed within an allocated rectangle
  // of a specified width and height.  The difference between allocation size
  // and doc size in each dimension is the gap between drawn documents.
  protected $alloc_width;
  protected $alloc_height;

  // Margins from the edge of the page to the top left corner of the first
  // document on the page.
  protected $left_margin;
  protected $top_margin;

  protected $n_up;  // Number of documents on one page
  protected $n_per_row;  // Number of documents in each row on the page

  protected $count;

  public function SetHorizontalGap($gap) {
    $this->alloc_width = $this->doc_width + $gap;
    $this->CalculateMargins();
  }

  public function SetVerticalGap($gap) {
    $this->alloc_height = $this->doc_height + $gap;
    $this->CalculateMargins();
  }

  public function SetLeftMargin($left) {
    $this->left_margin = $left;

    $n_per_column = $this->n_up / $this->n_per_row;  // Must be an int

    $printable_width = $this->pdf->GetPageWidth() - 2 * $left;
    $this->n_per_row = floor($printable_width / $this->doc_width);
    $gap = ($printable_width - $this->n_per_row * $this->doc_width)
        / ($this->n_per_row - 1);
    $this->alloc_width = $this->doc_width + $gap;

    $this->n_up = $this->n_per_row * $n_per_column;
  }

  public function SetTopMargin($top) {
    $this->top_margin = $top;

    $printable_height = $this->pdf->GetPageHeight() - 2 * $top;
    $n_per_column = floor($printable_height / $this->doc_height);
    $gap = ($printable_height - $n_per_column * $this->doc_height) / ($n_per_column - 1);
    $this->alloc_height = $this->doc_height + $gap;

    $this->n_up = $this->n_per_row * $n_per_column;
  }

  // For diagnosing layout problems
  public function GetParameters() {
    return array('alloc' => array('width' => $this->alloc_width,
                                  'height' => $this->alloc_height),
                 'margins' => array('left' => $this->left_margin,
                                    'top' => $this->top_margin),
                 'gap' => array('h' => $this->alloc_width - $this->doc_width,
                                'v' => $this->alloc_height - $this->doc_height),
                  );
  }

  // Returns array{ x=>, y=> } of top left corner coordinators for the next document
  public function NextTopLeft() {
    // $on_page is the index of the next document on the current page.
    $on_page = $this->count % $this->n_up;
    ++$this->count;

    if ($on_page == 0) {
      $this->pdf->AddPage();
      $this->pdf->SetMargins(0, 0, 0);
      $this->pdf->SetAutoPageBreak(false);
    }

    $row = floor($on_page / $this->n_per_row);
    $col = $on_page % $this->n_per_row;

    return array('x' => $this->left_margin + $col * $this->alloc_width,
                 'y' => $this->top_margin  + $row * $this->alloc_height);
  }
}

?>