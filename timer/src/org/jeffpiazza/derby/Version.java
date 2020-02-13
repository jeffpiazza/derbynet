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
    try {
      Enumeration<URL> resources = Version.class.getClassLoader()
          .getResources("META-INF/MANIFEST.MF");
      while (resources.hasMoreElements()) {
        Manifest manifest = new Manifest(resources.nextElement().openStream());
        Attributes derbynetAttrs = manifest.getAttributes("derbynet");
        if (derbynetAttrs != null) {
          return derbynetAttrs.getValue("version");
        }
      }
    } catch (IOException ex) {
      Logger.getLogger(Version.class.getName()).log(Level.SEVERE, null, ex);
    }
    return "Not found";  // TODO
  }
}
