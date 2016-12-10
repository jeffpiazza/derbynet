package org.jeffpiazza.derby;

import java.io.ByteArrayInputStream;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.net.*;

public class ClientSession {
  private CookieManager manager;
  private String base_url;

  public ClientSession(String base_url) {
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
  }

  public Element login(String username, String password) throws IOException {
    return doPost("action.php",
                  "action=login&name=" + username + "&password=" + password);
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
    OutputStreamWriter writer = new OutputStreamWriter(connection.
        getOutputStream());
    writer.write(params);
    writer.flush();
    writer.close(); // writer.close() may block.

    return getResponse(connection);
  }

  public Element doQuery(String q) throws IOException {
    return doQuery(new URL(base_url + "action.php?query=" + q));
  }

  public Element doQuery(String q, String params) throws IOException {
    return doQuery(new URL(base_url + "action.php?query=" + q + "&" + params));
  }

  public Element doQuery(URL url) throws IOException {
    HttpURLConnection connection = (HttpURLConnection) url.openConnection();
    connection.setRequestMethod("GET");
    return getResponse(connection);
  }

  private Element getResponse(HttpURLConnection connection) throws IOException {
    // This code will block until a response is actually received.  That should
    // be OK as long as there's a thread dedicated to handling requests in
    // this session.
    if (connection.getResponseCode() == 200) {
      return parseResponse(connection.getInputStream());
    }

    return null;
  }

  protected Element parseResponse(String s) throws IOException {
    return parseResponse(new ByteArrayInputStream(s.getBytes("UTF-8")));
  }

  protected Element parseResponse(final InputStream inputStream)
      throws IOException {
    inputStream.mark(10000);
    try {
      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      DocumentBuilder db = dbf.newDocumentBuilder();
      try {
        return db.parse(inputStream).getDocumentElement();
      } catch (Exception e) {
        System.err.println("Failed");
        e.printStackTrace();
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
    System.out.
        println(Timestamp.string() + ": Unparseable response for message");
    inputStream.reset();
    byte[] buffer = new byte[1024];
    int bytesRead;
    while ((bytesRead = inputStream.read(buffer)) != -1) {
      System.out.write(buffer, 0, bytesRead);
    }
    return null;
  }

  public static boolean wasSuccessful(Element response) {
    if (response == null) {
      return false;
    } else if (response.getElementsByTagName("success").getLength() == 0
        || response.getElementsByTagName("failure").getLength() > 0) {
      System.out.println(Timestamp.string() + ": Message resulted in failure");
      System.out.println("=======================");
      System.out.println(XmlSerializer.serialized(response));
      System.out.println("=======================");
      return false;
    } else {
      return true;
    }
  }
}
