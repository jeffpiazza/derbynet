// fakeajax module

const assert = require('./assert.js');

var fakeAjax = {
  _expected_config: {},
  _return_value: false,
  _pending: false,  // A Promise that resolves when expectation is met.
  _resolve: false,  // The resolve function for the _pending promise
  _reject: false,   // The reject function for the _pending promise
  _timeout_id: false,  // The id for a pending timeout that will
                       // fail the promise if not fulfilled in time

  _debugging: false,
  setDebugging: function(d) { this._debugging = d; },

  onAjax: function(url, config) {
    if (this._debugging) {
      console.log("onAjax fires with " + JSON.stringify(config));
    }
    assert.equal(this._expected_config, config);
    clearTimeout(this._timeout_id);
    this._resolve(true);
    return this._return_value;
  },

  expect: function(ex_config, ret_value) {
    this._expected_config = ex_config;
    this._return_value = ret_value;
    this._pending = new Promise(function(resolve, reject) {
      fakeAjax._resolve = resolve;
      fakeAjax._reject = reject;
    });
    this._timeout_id = setTimeout(() => {
      fakeAjax._reject('fakeAjax timed out waiting for ' + JSON.stringify(ex_config));
    },
                                  30000);
  },

  completion: function() {
    return this._pending;
  },

  installOn: async function(page) {
    await page.evaluate(() => {
      $.ajax = async function (url, config) {
        var xml = await window.onAjax(url, config); 
        if (xml) {
          config.success((new DOMParser()).parseFromString(xml, 'text/xml'));
        }
      };
    });

    await page.exposeFunction('onAjax', (url, config) => {
      return fakeAjax.onAjax(url, config);
    });
  },

  testForAjax: async function(actions, expected_call, return_value) {
    this.expect(expected_call, return_value);
    await actions();
    await this.completion();
  }
};

module.exports = fakeAjax;
