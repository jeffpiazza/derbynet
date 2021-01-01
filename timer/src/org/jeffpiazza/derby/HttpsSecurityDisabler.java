package org.jeffpiazza.derby;

import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

public class HttpsSecurityDisabler {
  public static void disableHttpsSecurity() {
    try {
      SSLContext sc = SSLContext.getInstance("SSL");
      sc.init(null,
              new TrustManager[]{
                new X509TrustManager() {
              public X509Certificate[] getAcceptedIssuers() {
                return null;
              }

              public void checkClientTrusted(X509Certificate[] certs,
                                             String authType) {
              }

              public void checkServerTrusted(X509Certificate[] certs,
                                             String authType) {
              }
            }}, new SecureRandom());
      HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());

      HttpsURLConnection.setDefaultHostnameVerifier(new HostnameVerifier() {
        public boolean verify(String hostname, SSLSession session) {
          return true;
        }
      });
    } catch (KeyManagementException ex) {
      Logger.getLogger(ClientSession.class.getName()).
          log(Level.SEVERE, null, ex);
    } catch (NoSuchAlgorithmException ex) {
      Logger.getLogger(ClientSession.class.getName()).
          log(Level.SEVERE, null, ex);
    }
  }
}
