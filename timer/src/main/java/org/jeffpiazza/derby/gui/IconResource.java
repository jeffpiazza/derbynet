package org.jeffpiazza.derby.gui;

public enum IconResource {
  TROUBLE,
  UNKNOWN,
  OK;

  public static final String mainPath = "/status";

  static IconResource getByName(String name) {
    return IconResource.valueOf(name.toUpperCase());
  }

  public String getPath() {
    String filename = String.join(".", name(), "png");
    return String.join("/", mainPath, filename);
  }
}
