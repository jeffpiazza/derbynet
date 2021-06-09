package org.jeffpiazza.derby;

import java.io.IOException;
import java.util.ArrayList;
import org.jeffpiazza.derby.gui.TimerGui;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
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
      JSONObject roles_result = session.doJsonQueryWithVariations("role.list");
      if (roles_result == null) {
        client.roleFinderFailed(
            "No response, or response not understood (likely wrong URL)");
      } else {
        synchronized (this) {
          serverAddress = session.getBaseUrl();
        }
        JSONArray rolesj = roles_result.getJSONArray("roles");
        if (rolesj.length() == 0) {
          client.roleFinderFailed(
              "Check the database; no roles provided in roles query");
        } else {
          for (int i = 0; i < rolesj.length(); ++i) {
            JSONObject role = rolesj.getJSONObject(i);
            if (role.optInt("timer-message", 0) != 0 ||
                role.optInt("race-control", 0) != 0) {
              roles.add(role.getString("name"));
            }
          }
          if (roles.isEmpty()) {
            client.roleFinderFailed("Roles received, but none are suitable.");
          }
        }
      }
    } catch (JSONException je) {
      client.roleFinderFailed(je.getMessage());
      LogWriter.stacktrace(je);
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
