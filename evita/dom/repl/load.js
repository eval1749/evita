// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

$define(global, 'repl', function($export) {
  const LINE_COMMENT = '\x2F/';

  /**
   * @param {string} scriptPath
   * @param {!Object=} opt_options
   * @return {!Promise}
   *
   * Options:
   *  encoding: string
   *  verbose: boolean
   */
  function load(scriptPath, opt_options) {
    let options = opt_options ? /** @type{!Object} */(opt_options) : {};
    let encoding = options.encoding ? /** @type{string} */(options.encoding)
        : 'utf-8';
    if (options.verbose)
      console.log(LINE_COMMENT, 'loading', scriptPath);
    /** @type {Os.File} */
    let file = null;
    return (async(function*() {
      file = yield Os.File.open(scriptPath);
      /** @type {number} */
      let total_read = 0;
      /** @type {string} */
      let text = '';
      /** @type {!ArrayBufferView} */
      const buffer = new Uint8Array(4096);
      /** @type {!TextDecoder} */
      const decoder = new TextDecoder(encoding, {fatal: true});
      for (;;) {
        let num_read = yield file.read(buffer);
        if (num_read == 0)
          break;
        text += decoder.decode(buffer.subarray(0, num_read));
        total_read += num_read;
      }
      file.close();
      file = null;
      text = text.replace(/\r\n/g, '\n');
      if (options.verbose)
        console.log(LINE_COMMENT, 'loaded', scriptPath, total_read);
      let result = Editor.runScript(text, scriptPath);
      if (result.exception instanceof SyntaxError) {
        const syntaxError = result.exception + '; ' +
          scriptPath + '(' + result.lineNumber + ':' +
          result.startColumn + ':' + result.endColumn + ')';
        throw syntaxError;
      }
      if (result.exception)
        throw result.exception;
      return Promise.resolve();
    }))().catch((reason) => {
      if (file)
        file.close();
      throw reason;
    });
  }

  $export({load});
});

global.load = repl.load;

