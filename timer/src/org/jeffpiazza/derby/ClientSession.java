package org.jeffpiazza.derby;

import java.io.*;
import java.net.*;
import java.util.*;

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

        doPost("login-action.php", "name=" + username + "&password=" + password);
    }

    public String sendTimerMessage(String messageAndParams) throws IOException {
        return doPost("action=timer-message&" + messageAndParams);
    }

    public String doPost(String params) throws IOException {
        return doPost("action.php", params);
    }

    public String doPost(String url_path, String params) throws IOException {
        return doPost(new URL(base_url + url_path), params);
    }

    public String doPost(URL url, String params) throws IOException {
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
			BufferedReader in = new BufferedReader(
                new InputStreamReader(connection.getInputStream()));
			String inputLine;
			StringBuffer response = new StringBuffer();
 
			while ((inputLine = in.readLine()) != null) {
				response.append(inputLine + "\n");
			}
			in.close();
			return response.toString();
		} else {
            // TODO: throw
			return responseCode + "";
		}
	}
}
