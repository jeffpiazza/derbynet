require('./preamble');

page.open(root + 'coordinator.php', function(status) {
  console.log('Landed on ' + page.url + ': ' + status);
  assert.equal(5, page.evaluate(function() { return $(".scheduling_control").length; }));
  phantom.exit();
});
