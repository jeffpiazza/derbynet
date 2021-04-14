package org.jeffpiazza.derby;

import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.drafts.Draft;
import org.java_websocket.handshake.ServerHandshake;
import org.json.JSONException;
import org.json.JSONObject;

public class ObsClient extends WebSocketClient {
  public ObsClient(URI serverUri, Draft draft) {
    super(serverUri, draft);
  }

  public ObsClient(URI serverURI) {
    super(serverURI);
  }

  public ObsClient(URI serverUri, Map<String, String> httpHeaders) {
    super(serverUri, httpHeaders);
  }

  private void makeAuthenticationResponse(String challenge, String salt) {
    try {
      MessageDigest sha256 = MessageDigest.getInstance("SHA-256");
      Base64.Encoder base64 = Base64.getEncoder();

      String text = Flag.obs_password.value() + salt;
      String secret = base64.encodeToString(
          sha256.digest(text.getBytes(StandardCharsets.UTF_8)));

      String response_string = secret + challenge;
      String response = base64.encodeToString(
          sha256.digest(response_string.getBytes(StandardCharsets.UTF_8)));

      send(new JSONObject()
          .put("request-type", "Authenticate")
          .put("message-id", "auth-response")
          .put("auth", response)
          .toString());
      System.err.println("Authenticate message send");
    } catch (NoSuchAlgorithmException ex) {
      LogWriter.stacktrace(ex);
    }
  }

  @Override
  public void onOpen(ServerHandshake sh) {
    System.err.println("ObsClient.onOpen: " + sh);
    LogWriter.info("ObsClient.onOpen: " + sh);
    send(new JSONObject()
        .put("request-type", "GetAuthRequired")
        .put("message-id", "get-auth-required")
        .toString());
  }

  @Override
  public void onMessage(String string) {
    try {
      JSONObject jsonObject = new JSONObject(string);
      if ("get-auth-required".equals(jsonObject.getString("message-id"))) {
        if (jsonObject.has("authRequired")
            && jsonObject.getBoolean("authRequired")) {
          makeAuthenticationResponse(jsonObject.getString("challenge"),
                                     jsonObject.getString("salt"));
        } else {
          System.err.println("No auth required");
        }
      } else if ("auth-response".equals(jsonObject.getString("message-id"))) {
        System.err.println("Received response to auth-response: " + string);
      } else {
        System.err.println("Received response for " + jsonObject.getString("message-id"));
      }
    } catch (JSONException ex) {
      LogWriter.stacktrace(ex);
    }
  }

  @Override
  public void onClose(int i, String string, boolean bool) {
    System.err.println(
        "ObsClient.onClose(" + i + ", \"" + string + "\", " + bool + ")");
    LogWriter.info(
        "ObsClient.onClose(" + i + ", \"" + string + "\", " + bool + ")");
  }

  @Override
  public void onError(Exception exception) {
    System.err.println("ObsClient.onError: " + exception);
    LogWriter.info("ObsClient.onError: " + exception);
  }
}
