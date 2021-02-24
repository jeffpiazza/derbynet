package org.jeffpiazza.derby;

import java.io.IOException;
import java.util.ArrayList;
import org.jeffpiazza.derby.gui.TimerGui;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

// From a server address, tries to make first contact with the web server by
// asking about the available roles.
//
public class RoleFinder {
  public interface RoleFinderClient {
    public void rolesFound(ArrayList<String> roles, ClientSession session);
    // roleFinderFailed is responsible for logging, too.
    public void roleFinderFailed(String reason);
  }

  public static RoleFinder start(final String serverAddress,
                                 RoleFinderClient client) {
    final RoleFinder roleFinder = new RoleFinder(serverAddress, client);
    (new Thread() {
      @Override
      public void run() {
        roleFinder.findRoles();
      }
    }).start();
    return roleFinder;
  }

  // serverAddress and session are fixed for the lifetime of the RoleFinder,
  // but gui may be set to null by a cancel() call from another thread.
  private String serverAddress;
  private RoleFinderClient client;
  private ClientSession session;

  private RoleFinder(String serverAddress, RoleFinderClient client) {
    this.serverAddress = serverAddress;
    this.client = client;
    this.session = new ClientSession(serverAddress);
  }

  public synchronized String getServerAddress() {
    return serverAddress;
  }

  public ClientSession getSession() {
    return session;
  }

  public synchronized void cancel() {
    this.client = null;
  }

  // Spawn a new thread to run RoleFinder.findRoles.
  // On success, calls gui.addRole as needed, then gui.rolesComplete(true).
  // On failure, calls gui.rolesComplete(false)
  public void findRoles() {
    ArrayList<String> roles = new ArrayList<String>();
    try {
      Element roles_result = session.doQueryWithVariations("roles");
      if (roles_result == null) {
        client.roleFinderFailed(
            "No response, or response not understood (likely wrong URL)");
      } else {
        synchronized (this) {
          serverAddress = session.getBaseUrl();
        }
        NodeList roleNodes = roles_result.getElementsByTagName("role");
        if (roleNodes.getLength() == 0) {
          NodeList titles = roles_result.getElementsByTagName("title");
          if (titles.getLength() == 1 && titles.item(0).getFirstChild().
              getNodeValue().contains("Set-Up")) {
            // Redirected to the set-up page, because there's no database
            client.roleFinderFailed("Set up the server database before proceeding");
          } else {
            client.roleFinderFailed("No roles provided in roles query");
          }
        } else {
          for (int i = 0; i < roleNodes.getLength(); ++i) {
            Element role = (Element) roleNodes.item(i);
            if (!role.getAttribute("timer_message").isEmpty()) {
              roles.add(role.getTextContent());
            }
            if (!role.getAttribute("race_control").isEmpty()) {
              roles.add(role.getTextContent());
            }
          }
          if (roles.isEmpty()) {
            client.roleFinderFailed("Roles received, but none are suitable.");
          }
        }
      }
    } catch (ClientSession.HttpException he) {
      client.roleFinderFailed(he.getBriefMessage());
      LogWriter.info("RoleFinder failed: " + he.getMessage());
    } catch (IOException e) {
      client.roleFinderFailed(e.getMessage());
      LogWriter.info("RoleFinder failed: " + e.getMessage());
    } finally {
      synchronized (this) {
        if (!roles.isEmpty() && client != null) {
          client.rolesFound(roles, session);
        }
      }
    }
  }
}
