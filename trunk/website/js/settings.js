function on_form_submission() {
  console.log('submitted!');
  console.log($("#settings_form").serialize());
    $.ajax('action.php',
           {type: 'POST',
            data: $("#settings_form").serialize(),
            success: function(data) {
               var fail = data.documentElement.getElementsByTagName("failure");
               if (fail && fail.length > 0) {
                 console.log(data);
                 alert("Action failed: " + fail[0].textContent);
               } else {
                 window.location.href = "index.php";
               }
             },
             error: function(jqXHR, ajaxSettings, thrownError) {
               alert('Ajax error: ' + thrownError);
             }
           });
  return false;
};

$(function() { $("#settings_form").on("submit", on_form_submission); });
