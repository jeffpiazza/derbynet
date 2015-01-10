// Approximate structure for the photo-thumbs page:
//
// <body>
//   (refresh button)
//   <div body-wrapper>
//      <div thumblist>
//        <ul>
//           <li>...   -- has without-photo class if accepting new photos
//        </ul>
//      </div>
//
//      <div photothumbs>
//         <div thumbnail>
//            <a>
//               <img unassigned-photo />
//            </a>...
//         </div>
//      </div>
//   </div>
// </body>

// draggable and droppable elements with scope: "assign" are for
// dragging unassigned photos to racers who are still without a photo.
//
// scope: "undo" is for removing a photo assignment, by dragging from
// the list element back to the photothumbs div.

  // target is a jquery for list items that should be made assignable,
  // i.e., can receive new photo assignments.
  function make_assignable(target) {
	  target.droppable({
		scope: "assign",
		hoverClass: "droppableHover",
		drop: function(event, ui) {
			var photo_base_name = ui.draggable.attr("data-image-filename");
			var racer_id = $(this).attr("data-racer-id");

			changeRacerPhotoAjax(0, racer_id, photo_base_name);

			ui.draggable.closest(".thumbnail").addClass("hidden");
			$(this).prepend('<img class="assigned"' +
							' data-image-filename="' + photo_base_name + '"' +
							' src="photo.php/tiny/' +
                            'q' + Date.now() + '/' +
							encodeURIComponent(photo_base_name) + '"/>');

			make_discardable($(this).find(".assigned"));
			// Once dropped, make no longer droppable!
			$(this).droppable("destroy");
		  }
		});
  }

  function make_discardable(target) {
	target.draggable({
		    revert: 'invalid',
			scope: 'undo',

		    helper: 'clone',
		    appendTo: 'body',
		    opacity: 0.5,
			});
  }

  $(function() {
	  make_assignable($(".without-photo"));
	  $(".unassigned-photo").draggable({
		    helper: 'clone',
			appendTo: 'body',
			// When using helper: clone, the clone's top left corner
			// appears to be what controls selection of the droppable
			// target, and I don't see any way to change it to use the
			// mouse cursor instead.  If you can't beat 'em, join 'em:
			// use cursorAt to force the clone's top to match the
			// cursor.  (For picking from the list of racers, we don't
			// care so much about horizontal position.)

			// TODO: Don't know why only top has effect here.
			cursorAt: { top: 0, left: 20 },
			opacity: 0.5,
		    revert: 'invalid',
			scope: "assign",
			});

	  $(".photothumbs").droppable({
		    hoverClass: 'droppableHover',
			scope: 'undo',
			drop: function(event, ui) {
			  // ui.draggable will be the assigned <img> itself
			  var image_filename = ui.draggable.attr("data-image-filename");
			  console.log("discarded image filename = " + image_filename);

			  var list_item = ui.draggable.closest("li");
			  list_item.addClass("without-photo");
			  make_assignable(list_item);

			  ui.draggable.remove();
			  var hidden_thumbnail = $("img[data-image-filename='" + image_filename + "']").closest(".thumbnail");
			  hidden_thumbnail.removeClass("hidden");
			  // A previous drag may have added a style property with
			  // top and left values that will leave the image out of
			  // place; we need to clear them.
			  hidden_thumbnail.find("img")
				.css("top", "")
				.css("left", "");
			  removeRacerPhoto(list_item.attr("data-racer-id"));
		  }
		});

	  make_discardable($(".assigned"));
	});


function changeRacerPhotoAjax(previous, racer, photo) {
  console.log('changeRacerPhotoAjax: previous racer = ' + previous
			  + ', new racer = ' + racer + ', photo = ' + photo);

   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", g_action_url, /*async*/true);
   xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
// TODO   xmlhttp.onreadystatechange = readystate_handler;
   xmlhttp.send("action=photo&racer=" + racer
				+ "&previous=" + previous
				+ "&photo=" + photo);
// TODO   ajax_add_request();
}

function removeRacerPhoto(previous) {
  changeRacerPhotoAjax(previous, 0, '');
}
