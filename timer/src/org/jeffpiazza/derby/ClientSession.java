package org.jeffpiazza.derby;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.*;

public class ClientSession {
    private CookieManager manager;
    private String base_url;

    public ClientSession(String base_url, String username, String password) throws IOException {
        if (!base_url.startsWith("http://")) {
            base_url = "http://" + base_url;
        }
        if (!base_url.endsWith("/")) {
            base_url += "/";
        }
        manager = new CookieManager();
        manager.setCookiePolicy(CookiePolicy.ACCEPT_ALL);
        CookieHandler.setDefault(manager);
        this.base_url = base_url;

        doPost("action.php", "action=login&name=" + username + "&password=" + password);
    }

    public Element sendTimerMessage(String messageAndParams) throws IOException {
        return doPost("action=timer-message&" + messageAndParams);
    }

    public Element doPost(String params) throws IOException {
        return doPost("action.php", params);
    }

    public Element doPost(String url_path, String params) throws IOException {
        return doPost(new URL(base_url + url_path), params);
    }

    public Element doPost(URL url, String params) throws IOException {
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();
        connection.setRequestMethod("POST");

        connection.setDoOutput(true);
        OutputStreamWriter writer = new OutputStreamWriter(connection.getOutputStream());
        writer.write(params);
        writer.flush();
        writer.close();

        // This code (either the getResponseCode call itself, or maybe
        // the writer.close(), above) will block until a response is
        // actually received.  That should be OK as long as there's a
        // thread dedicated to handling requests in this session.
        int responseCode = connection.getResponseCode();

        if (responseCode == 200) {
            try {
                DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
                DocumentBuilder db = dbf.newDocumentBuilder();
                Document doc = db.parse(connection.getInputStream());
                System.out.println(XmlSerializer.serialized(doc.getDocumentElement()));  // TODO
                return doc.getDocumentElement();
            } catch (Exception e) {
                e.printStackTrace();  // TODO
            }
        }
        return null;
    }
}
