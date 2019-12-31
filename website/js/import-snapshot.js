function onFormSubmit(e) {
  console.log("onFormSubmit");
  e.preventDefault();
  $.ajax({
    url: "action.php",
    type: "POST",
    data:  new FormData(this),
    contentType: false,
    processData:false,
    success: function(data) {
      var succ = data.documentElement.getElementsByTagName("success");
      if (succ && succ.length > 0) {
        window.location = "setup.php";
      }
   },
  });
}

function enableOrDisableSubmitButton() {
  var hasFile = $("#snapshot_file").val() != '';
  $("#submit-snapshot").prop('disabled', !hasFile);
  $("#upload-form label").toggleClass('hidden', hasFile);
  $(".file_target").toggleClass('draghover', hasFile);
  $("#filepath").toggleClass('hidden', !hasFile);
  if (hasFile) {
    $("#filepath").text($("#snapshot_file")[0].files[0].name);
  }
}

$(function() {

  $(".file_target input").on('dragenter', function() {
    // Event is received by the <input> element, whose parent is the
    // .file_target div
    $(event.target).parent().addClass("draghover");
  });
  $(".file_target input").on('dragleave', function() {
    $(event.target).parent().removeClass("draghover");
  });
  $(".file_target input").on('change', enableOrDisableSubmitButton);

  enableOrDisableSubmitButton();

  $("#upload-form").on("submit", onFormSubmit);
});

