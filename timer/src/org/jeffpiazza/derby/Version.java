package org.jeffpiazza.derby;

import java.io.IOException;
import java.net.URL;
import java.util.Enumeration;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Version {
  public static String get() {
    if (fetched_version == null) {
      getFresh();
    }
    return fetched_version;
  }

  public static String series() {
    if (fetched_series == null) {
      getFresh();
    }
    return fetched_series;
  }

  public static String revision() {
    if (fetched_revision == null) {
      getFresh();
    }
    return fetched_revision;
  }

  private static String fetched_version = null;
  private static String fetched_series = null;
  private static String fetched_revision = null;

  private static void getFresh() {
    try {
      Enumeration<URL> resources = Version.class.getClassLoader()
          .getResources("META-INF/MANIFEST.MF");
      while (resources.hasMoreElements()) {
        Manifest manifest = new Manifest(resources.nextElement().openStream());
        Attributes derbynetAttrs = manifest.getAttributes("derbynet");
        if (derbynetAttrs != null) {
          fetched_version = derbynetAttrs.getValue("version");
          fetched_revision = derbynetAttrs.getValue("revision");
          fetched_series = derbynetAttrs.getValue("series");
          return;
        }
      }
    } catch (IOException ex) {
      Logger.getLogger(Version.class.getName()).log(Level.SEVERE, null, ex);
    }
    fetched_version = fetched_revision = "Not found";
  }
}
