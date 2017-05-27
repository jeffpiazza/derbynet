require("./preamble");

(function() {
  var all_pages = [
     "about.php",
    "awards-editor.php",
    "awards-presentation.php",
    "checkin.php",
    "class-editor.php",
    "coordinator.php",
    // "export.php",
    "import-awards.php",
    "import-results.php",
    "import-roster.php",
    "index.php",
    "kiosk-dashboard.php",
    "login.php",
    "ondeck.php",
    "photo-thumbs.php",
    "racer-results.php",
    "settings.php",
    "setup.php",
    "standings.php"
  ];

  var next = function() {
    if (all_pages.length == 0) {
      phantom.exit(0);
    } else {
      var p = all_pages.shift();
      page.open(root + p, function(status) {
        assert.success(status);
        console.log("  " + "  " + "  " + scriptName + ": " + p);
        // setTimeout delays opening the next page by a "short" time, in an effort to avoid
        // problems with in-flight ajax calls getting canceled.
        setTimeout(function() { next(); }, 500);
      });
    }
  };

  next();
})();
