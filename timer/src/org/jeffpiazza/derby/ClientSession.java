package org.jeffpiazza.derby;

import java.io.ByteArrayInputStream;
import org.w3c.dom.Element;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

public class ClientSession {
  private CookieManager manager;
  private String base_url;
  private String original_base_url;

  private static final List<String> kTimerLogHeaders = new ArrayList<String>(
      Arrays.asList("Content-Type", "text/plain"));

  public static class HttpException extends IOException {
    public HttpException(int responseCode, String responseMessage, URL url) {
      this.responseCode = responseCode;
      this.responseMessage = responseMessage;
      this.url = url;
    }

    public HttpException(HttpURLConnection connection) throws IOException {
      this(connection.getResponseCode(), connection.getResponseMessage(),
           connection.getURL());
    }

    public final int responseCode;
    public final String responseMessage;
    public final URL url;

    public String getMessage() {
      return "HTTP response " + responseCode + " (" + responseMessage
          + ") for " + url;
    }

    public String getBriefMessage() {
      return "HTTP response " + responseCode + " (" + responseMessage + ")";

    }
  }

  public ClientSession(String base_url) {
    LogWriter.info("Attempting connection to " + base_url);
    String lowercase_url = base_url.toLowerCase();
    if (!lowercase_url.startsWith("http://")
        && !lowercase_url.startsWith("https://")) {
      base_url = "http://" + base_url;
    }
    if (!base_url.endsWith("/")) {
      base_url += "/";
    }
    original_base_url = base_url;
    manager = new CookieManager();
    manager.setCookiePolicy(CookiePolicy.ACCEPT_ALL);
    CookieHandler.setDefault(manager);
    this.base_url = base_url;
  }

  public String getBaseUrl() {
    return base_url;
  }

  // Repeated calls to makeUrlVariation should rewrite base_url to different
  // variations of the original URL, or return false if there are no more.
  // This implementation considers only one variation.
  private boolean makeUrlVariation() {
    if (base_url.equals(original_base_url)) {
      if (base_url.endsWith("/derbynet/")) {
        base_url = base_url.substring(0, base_url.lastIndexOf("derbynet/"));
      } else {
        base_url = base_url + "derbynet/";
      }
      System.err.println();
      System.err.println("Trying URL variation " + base_url);
      return true;
    }
    return false;
  }

  public JSONObject login(String role, String password) throws IOException {
    return doJsonPostWithVariations("action.php",
                                "action=role.login&name=" + role
                                + "&password=" + password);
  }

  public Element sendTimerMessage(String messageAndParams) throws IOException {
    return doActionPost("action=timer-message&" + messageAndParams);
  }

  public void sendTimerLogFragment(String logFragment) throws IOException {
    // Ignore the response from the timer log POST
    doTimerLogPost(logFragment);
  }

  private Element doActionPost(String body) throws IOException {
    return doPost("action.php", null, body);
  }

  private Element doTimerLogPost(String body) throws IOException {
    return doPost("post-timer-log.php", kTimerLogHeaders, body);
  }

  private Element doPostWithVariations(String url_path, String body)
      throws IOException {
    return makeRequestWithVariations(url_path, "POST", null, body);
  }

  private JSONObject doJsonPostWithVariations(String url_path, String body)
      throws IOException {
    return makeJsonRequestWithVariations(url_path, "POST", null, body);
  }

  private URL fullUrl(String url_path) throws MalformedURLException {
    return new URL(base_url + url_path);
  }

  private Element doPost(String url_path, List<String> headers, String body)
      throws IOException {
    return doPost(fullUrl(url_path), headers, body);
  }

  private Element doPost(URL url, List<String> headers, String body)
      throws IOException {
    return makeRequest(url, "POST", headers, body);
  }

  public Element doQueryWithVariations(String q) throws IOException {
    return makeRequestWithVariations("action.php?query=" + q, "GET", null, null);
  }

  public JSONObject doJsonQueryWithVariations(String q) throws IOException {
    return makeJsonRequestWithVariations("action.php?query=" + q,
                                         "GET", null, null);
  }

  // Overridden by SimulatedClientSession
  protected Element doQuery(URL url) throws IOException {
    return makeRequest(url, "GET", null, null);
  }

  public Element makeRequestWithVariations(String url_path, String method,
                                           List<String> headers, String body)
      throws IOException {
    Element result = null;
    HttpException firstException = null;

    try {
      result = makeRequest(fullUrl(url_path), method, headers, body);
    } catch (HttpException he) {
      firstException = he;
    }

    while (result == null && makeUrlVariation()) {
      try {
        result = makeRequest(fullUrl(url_path), method, headers, body);
      } catch (HttpException he) {
        LogWriter.info("Ignoring for variation: " + he.getMessage());
      }
    }

    if (result == null && firstException != null) {
      throw firstException;
    }

    return result;
  }

  public JSONObject makeJsonRequestWithVariations(String url_path, String method,
                                           List<String> headers, String body)
      throws IOException {
    JSONObject result = null;
    HttpException firstException = null;

    try {
      result = makeJsonRequest(fullUrl(url_path), method, headers, body);
    } catch (HttpException he) {
      firstException = he;
    }

    while (result == null && makeUrlVariation()) {
      try {
        result = makeJsonRequest(fullUrl(url_path), method, headers, body);
      } catch (HttpException he) {
        LogWriter.info("Ignoring for variation: " + he.getMessage());
      }
    }

    if (result == null && firstException != null) {
      throw firstException;
    }

    return result;
  }

  private Element makeRequest(URL url, String method, List<String> headers,
                              String body) throws IOException {
    HttpURLConnection connection;
    do {
      connection = (HttpURLConnection) url.openConnection();

      connection.setRequestMethod(method);
      connection.addRequestProperty("User-Agent",
                                    "derby-timer.jar/" + Version.get());
      if (headers != null) {
        for (int i = 0; i < headers.size(); i += 2) {
          connection.addRequestProperty(headers.get(i), headers.get(i + 1));
        }
      }

      if (body != null) {
        connection.setDoOutput(true);
        OutputStreamWriter writer
            = new OutputStreamWriter(connection.getOutputStream());
        writer.write(body);
        writer.flush();
        writer.close(); // writer.close() may block.
      }
    } while ((url = urlFromRedirection(connection)) != null);

    return getResponse(connection);
  }

  private JSONObject makeJsonRequest(URL url, String method, List<String> headers,
                              String body) throws IOException {
    HttpURLConnection connection;
    do {
      connection = (HttpURLConnection) url.openConnection();

      connection.setRequestMethod(method);
      connection.addRequestProperty("User-Agent",
                                    "derby-timer.jar/" + Version.get());
      if (headers != null) {
        for (int i = 0; i < headers.size(); i += 2) {
          connection.addRequestProperty(headers.get(i), headers.get(i + 1));
        }
      }

      if (body != null) {
        connection.setDoOutput(true);
        OutputStreamWriter writer
            = new OutputStreamWriter(connection.getOutputStream());
        writer.write(body);
        writer.flush();
        writer.close(); // writer.close() may block.
      }
    } while ((url = urlFromRedirection(connection)) != null);

    return getJsonResponse(connection);
  }

  private URL urlFromRedirection(HttpURLConnection connection) {
    try {
      if (300 <= connection.getResponseCode()
          && connection.getResponseCode() < 400) {
        String location = connection.getHeaderField("Location");
        LogWriter.info(
            "HTTP Redirect: " + connection.getResponseCode() + " ("
            + connection.getResponseMessage() + ") to " + location);

        maybeUpdateBaseUrlFromRedirection(connection.getURL(), location);
        return new URL(location);
      }
    } catch (Throwable ex) {
      LogWriter.info("Failed to process redirection: " + ex.getMessage());
    }

    return null;
  }

  private void maybeUpdateBaseUrlFromRedirection(
      URL url, String redirect_string) {
    String url_string = url.toString();
    if (!url_string.startsWith(base_url)) {
      LogWriter.info("Original URL " + url_string
          + " doesn't agree with base_url " + base_url);
      return;
    }
    String tail = url_string.substring(base_url.length());

    // Redirect may include query parameters for a GET
    if (redirect_string.endsWith(tail)) {
      base_url = redirect_string.substring(
          0, redirect_string.length() - tail.length());
      LogWriter.info("Updating base URL to " + base_url);
    } else {
      LogWriter.info("Original URL " + url_string
          + " doesn't agree with tail  of redirect " + redirect_string);
    }
  }

  private Element getResponse(HttpURLConnection connection) throws IOException {
    // This code will block until a response is actually received.  That should
    // be OK as long as there's a thread dedicated to handling requests in
    // this session.
    final int responseCode = connection.getResponseCode();
    if (responseCode == 200) {
      return parseResponse(connection.getInputStream());
    }

    throw new HttpException(connection);
  }

  private JSONObject getJsonResponse(HttpURLConnection connection) throws IOException {
    final int responseCode = connection.getResponseCode();
    if (responseCode == 200) {
      return parseJsonResponse(connection.getInputStream());
    }

    throw new HttpException(connection);
  }

  protected JSONObject parseJsonResponse(String s) throws IOException {
    return new JSONObject(s);
  }

  protected JSONObject parseJsonResponse(final InputStream inputStream)
      throws IOException {
    // TODO inputStream.mark(10000);
    return new JSONObject(new JSONTokener(new InputStreamReader(inputStream)));
  }

  protected Element parseResponse(String s) throws IOException {
    return parseResponse(new ByteArrayInputStream(s.getBytes("UTF-8")));
  }

  // For Login request, response may be json rather than xml
  protected Element parseResponse(final InputStream inputStream)
      throws IOException {
    inputStream.mark(10000);
    try {
      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      DocumentBuilder db = dbf.newDocumentBuilder();
      return db.parse(inputStream).getDocumentElement();
    } catch (Exception e) {
      LogWriter.stacktrace(e);
    }
    LogWriter.httpResponse("Unparseable response for message");
    inputStream.reset();
    byte[] buffer = new byte[1024];
    int bytesRead;
    while ((bytesRead = inputStream.read(buffer)) != -1) {
      System.out.write(buffer, 0, bytesRead);
      LogWriter.httpResponse(new String(buffer, 0, bytesRead));
    }
    return null;
  }

  public static boolean wasSuccessful(JSONObject response) {
    if (response == null) {
      System.err.println("null response");
      return false;
    }
    try {
      return response.getJSONObject("outcome").getString("summary").equals(
          "success");
    } catch (JSONException ex) {
      return false;
    }
  }

  public static boolean wasSuccessful(Element response) {
    if (response == null) {
      return false;
    } else if (response.getElementsByTagName("success").getLength() == 0
        || response.getElementsByTagName("failure").getLength() > 0) {
      LogWriter.httpResponse("Message resulted in failure");
      LogWriter.httpResponse(response);
      System.err.println(Timestamp.string() + ": Message resulted in failure");
      System.err.println("=======================");
      System.err.println(XmlSerializer.serialized(response));
      System.err.println("=======================");
      return false;
    } else {
      return true;
    }
  }
}
