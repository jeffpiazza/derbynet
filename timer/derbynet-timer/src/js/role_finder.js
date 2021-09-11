'use strict';

// HostPoller  holds url, but maybe that should break out into its own class.
// g_host_poller

class RoleFinder {

  roles = [];

  clear_roles() {
    this.roles = [];
  }
  
  async find_roles() {
    this.roles = [];
    try {
      var self = this;
      await $.ajax(HostPoller.url,
                   {data: {query: 'role.list'},
                    success: function(data) {
                      console.log('role.list returned with', data);
                      if ('roles' in data) {
                        for (var i = 0; i < data.roles.length; ++i) {
                          if (data.roles[i]['timer-message'] != 0) {
                            self.roles.push(data.roles[i]);
                          }
                        }
                      }
                    },
                    error: function(xhr, err, status) {
                      console.log('role_finder error function');
                      // result = 'failure';
                    }});
    } catch (e) {
      console.log('role_finder catch', e);
      // result = 'caught';
    }

    return this.roles;
  }
}
