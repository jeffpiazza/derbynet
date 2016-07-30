
// The polling rate for the page is relatively fast, and each time
// causes a rewrite of everything for all the kiosks.  If the user has
// opened the <select> element for choosing a page to present on a
// kiosk, rewriting the control will cause the user's action to be
// ignored.  To combat this, we only update the kiosk groups when
// there's a detectable change, and for that, we keep a hash of what
// the last state of the kiosk data was.
g_kiosk_hash = 0;

function hash_string(hash, str) {
	for (i = 0; i < str.length; i++) {
		var ch = str.charCodeAt(i);
		hash = ((hash<<5)-hash)+ch;
		hash = hash & hash; // Convert to 32bit integer
	}
	return hash;
}

function generate_kiosk_controls(pages, kiosks) {
    var hash = 0;
    for (var i = 0; i < kiosks.length; ++i) {
        var kiosk = kiosks[i];
        hash = hash_string(hash, kiosk.name);
        hash = hash_string(hash, kiosk.address);
        hash = hash_string(hash, kiosk.last_contact);
        hash = hash_string(hash, kiosk.assigned_page);
    }
    if (hash != g_kiosk_hash) {
        $("#kiosk_control_group").empty();
        for (var i = 0; i < kiosks.length; ++i) {
            generate_kiosk_control_group(i, kiosks[i], pages);
        }
        g_kiosk_hash = hash;
    }
}

// Controls for kiosks
// sel is the <select data-kiosk-address> input element
function handle_assign_kiosk_page_change(sel) {
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'kiosk.assign',
                   address: sel.getAttribute('data-kiosk-address'),
                   page: sel.value},
           });
}

function show_kiosk_naming_modal(address, name) {
    $("#kiosk_name_field").val(name);
    show_modal("#kiosk_modal", function(event) {
        handle_name_kiosk(address, $("#kiosk_name_field").val());
        return false;
    });
}

function handle_name_kiosk(address, name) {
    close_modal("#kiosk_modal");
    $.ajax(g_action_url,
           {type: 'POST',
            data: {action: 'kiosk.assign',
                   address: address,
                   name: name},
           });
}

function parse_kiosk_pages(data) {
    var kiosk_pages_xml = data.getElementsByTagName("kiosk-page");
    var kiosk_pages = new Array(kiosk_pages_xml.length);
    for (var i = 0; i < kiosk_pages_xml.length; ++i) {
        kiosk_pages[i] = {brief: kiosk_pages_xml[i].getAttribute('brief'),
                          path: kiosk_pages_xml[i].textContent};
    }
    return kiosk_pages;
}

function parse_kiosks(data) {
    var kiosks_xml = data.getElementsByTagName("kiosk");
    var kiosks = new Array(kiosks_xml.length);
    for (var i = 0; i < kiosks_xml.length; ++i) {
        var kiosk_xml = kiosks_xml[i];
        kiosks[i] = {name: kiosk_xml.getElementsByTagName("name")[0].textContent,
                     address: kiosk_xml.getElementsByTagName("address")[0].textContent,
                     last_contact: kiosk_xml.getElementsByTagName("last_contact")[0].textContent,
                     assigned_page: kiosk_xml.getElementsByTagName("assigned_page")[0].textContent};
    }
    return kiosks;
}

function generate_kiosk_control_group(index, kiosk, pages) {
    var div = $("<div class=\"block_buttons\"/>");
    var elt = $("<div class=\"control_group kiosk_control\"/>");

    elt.append("<p>Kiosk <span class=\"kiosk_control_name\"></span>"
               + " <span class=\"kiosk_control_address\"></span>"
               + "</p>");
    elt.find(".kiosk_control_name").text(kiosk.name);
    elt.find(".kiosk_control_address").text(kiosk.address);

    elt.append("<p class=\"last_contact\">Last contact: " + kiosk.last_contact + "</p>");
    elt.append("<label for=\"kiosk-page-" + index + "\">Display:</label>");
    var sel = $("<select name=\"kiosk-page-" + index + "\"" 
                + " data-kiosk-address=\"" + kiosk.address + "\"" 
                + " onchange=\"handle_assign_kiosk_page_change(this)\""
                + "/>");
    for (var i = 0; i < pages.length; ++i) {
        opt = $("<option value=\"" + pages[i].path + "\">" + pages[i].brief + "</option>");
        if (kiosk.assigned_page == pages[i].path) {
            opt.prop("selected", true);
        }
        sel.append(opt);
    }

    sel.appendTo(elt);
    elt.append('<input type="button" data-enhanced="true"'
               + ' onclick="show_kiosk_naming_modal(\''
               + kiosk.address.replace(/"/g, '&quot;').replace(/'/, "\\'")
               + '\', \'' + kiosk.name.replace(/"/g, '&quot;').replace(/'/, "\\'")
               + '\')"'
               + ' value="Assign Name"/>');

    elt.appendTo(div);
    div.appendTo("#kiosk_control_group");
}
