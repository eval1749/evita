// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  class TraceLogger {
    constructor() {
      /** @type {!Array.<!Object>} */
      this.events_ = [];
    }

    /** @return {undefined} */
    start() {
      this.events_ = [];
      Editor.startTraceLog('{"excluded_categories": []}');
    }

    /** @return {!Promise.<number>} */
    stop() {
      let resultResolver;
      const resultPromise =
          new Promise((resolver) => {resultResolver = resolver});
      let json = '\u005b';
      let delimiter = '';
      Editor.stopTraceLog((chunk, more) => {
        json += delimiter;
        json += chunk;
        delimiter = ',';
        if (more)
          return;
        json += '\u005d';
        this.events_ = /** @type {!Array.<Object>} */(JSON.parse(json));
        resultResolver(this.events_.length);
      });
      return resultPromise;
    }

    /** @return {!Promise.<?>} */
    writeTo(fileName) {
      /** @type {Os.File} */
      let file = null;
      /** @type {!TextEncoder} */
      const encoder = new TextEncoder('utf-8');
      return async(function*(events) {
         file = yield Os.File.open(fileName, 'w');
         const json = JSON.stringify(events);
         yield file.write(encoder.encode(json));
         file.close();
         return events.length;
       })(this.events_).catch((reason) => {
        if (!file)
          return;
        file.close();
        file = null;
      });
    }
  }

  global.TraceLogger = TraceLogger;
})();
